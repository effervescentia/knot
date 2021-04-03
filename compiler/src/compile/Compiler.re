open Kore;
open Reference;

type config_t = {
  name: string,
  root_dir: string,
  source_dir: string,
  fail_fast: bool,
};

type action_t =
  | Report(list(compile_err))
  | Flush;

/**
 parses and transforms programs represented as a graph of modules
 */
type t = {
  config: config_t,
  graph: ImportGraph.t,
  modules: ModuleTable.t,
  resolver: Resolver.t,
  dispatch: action_t => unit,
};

let __module_table_size = 64;

let _get_exports = ast =>
  ast
  |> List.filter_map(
       fun
       | AST.Declaration((Private(_), _), _) => None
       | AST.Declaration((id, _), decl) =>
         Some((id, decl |> TypeOf.declaration))
       | _ => None,
     );

/* static */

/**
 construct a new compiler instance
 */
let create = (~report=throw_all, config: config_t): t => {
  let cache = Cache.create(config.name);
  let resolver = Resolver.create(cache, config.root_dir, config.source_dir);

  let errors = ref([]);
  let dispatch =
    fun
    | Report(errs) =>
      if (config.fail_fast) {
        report(errs);
      } else {
        errors := errors^ @ errs;
      }
    | Flush => {
        let errs = errors^;

        if (!List.is_empty(errs)) {
          errors := [];
          report(errs);
        };
      };

  let graph =
    ImportGraph.create(id =>
      switch (
        resolver
        |> Resolver.resolve_module(~skip_cache=true, id)
        |> Module.read(Parser.imports)
      ) {
      | Ok(x) => x
      | Error(errs) =>
        Report(errs) |> dispatch;
        /* assume no imports if parsing failed */
        [];
      }
    );

  let modules = Hashtbl.create(__module_table_size);

  {dispatch, config, graph, modules, resolver};
};

/* methods */

let resolve = (~skip_cache=false, compiler, id) =>
  Resolver.resolve_module(~skip_cache, id, compiler.resolver);

/**
 check for import cycles and missing modules
 */
let validate = (compiler: t) => {
  [Validate.no_import_cycles, Validate.no_unresolved_modules]
  |> List.iter(validate =>
       compiler.graph
       |> validate
       |> Result.iter_error(errs => Report(errs) |> compiler.dispatch)
     );

  compiler.dispatch(Flush);
};

/**
 parse modules and add to table
 */
let process =
    (
      ids: list(Namespace.t),
      resolve: Namespace.t => Module.t,
      {modules} as compiler: t,
    ) => {
  ids
  |> List.iter(id =>
       switch (
         resolve(id)
         |> Module.read(
              Parser.ast(
                ~ctx=Context.create(~scope=Scope.create(~modules, ()), ()),
              ),
            )
       ) {
       | Ok(ast) => modules |> ModuleTable.add(id, ast, _get_exports(ast))
       | Error(errs) => Report(errs) |> compiler.dispatch
       }
     );

  compiler.dispatch(Flush);
};

/**
 fill import graph from entry and parse program to AST
 */
let init = (~skip_cache=false, entry: Namespace.t, compiler: t) => {
  compiler.graph |> ImportGraph.init(entry);
  compiler.dispatch(Flush);

  /* check if import graph is valid */
  compiler |> validate;

  let modules = ImportGraph.get_modules(compiler.graph);

  if (!skip_cache) {
    /* cache snapshot of modules on disk */
    modules
    |> List.iter(id =>
         switch (
           resolve(~skip_cache=true, compiler, id)
           |> Module.cache(compiler.resolver.cache)
         ) {
         | Ok(_) => ()
         | Error(errs) => Report(errs) |> compiler.dispatch
         }
       );
  };

  /* parse modules to AST */
  compiler |> process(modules, resolve(~skip_cache, compiler));
};

/**
 re-evaluate a subset of the import graph
 */
let incremental = (ids: list(Namespace.t), compiler: t) => {
  compiler |> validate;
  compiler |> process(ids, resolve(~skip_cache=true, compiler));

  /* generate output files */

  Log.info("incremental compilation successful");
};

/**
 use the ASTs of parsed modules to generate files in the target format
 */
let emit_output = (target: Target.t, output_dir: string, compiler: t) =>
  compiler.modules
  |> Hashtbl.iter(
       Namespace.(
         fun
         | Internal(name) =>
           Filename.concat(output_dir, name ++ Target.extension_of(target))
           |> (
             path => {
               let parent_dir = Filename.dirname(path);

               parent_dir |> FileUtil.mkdir(~parent=true);

               let out = open_out(path);

               (ModuleTable.{ast}) => {
                 Generator.generate(
                   target,
                   fun
                   | Internal(path) =>
                     Filename.concat(output_dir, path)
                     |> Filename.relative_to(parent_dir)
                   | External(_) => raise(NotImplemented),
                   ast,
                 )
                 |> Writer.write(out);
                 close_out(out);
               };
             }
           )
         | External(_) => raise(NotImplemented)
       ),
     );

/**
 compile the entire program to the target
 */
let compile =
    (target: Target.t, output_dir: string, entry: Namespace.t, compiler: t) => {
  compiler |> init(entry);

  FileUtil.rm(~recurse=true, [output_dir]);
  FileUtil.mkdir(~parent=true, output_dir);

  compiler |> emit_output(target, output_dir);

  /* generate output files */

  Log.info("compilation successful");
};

/**
 add a new module (and its import graph) to a compiler
 */
let add_module = (id: Namespace.t, compiler: t) =>
  if (compiler.graph |> ImportGraph.has_module(id)) {
    compiler.graph |> ImportGraph.get_imported_by(id);
  } else {
    compiler.graph |> ImportGraph.add_module(id);
  };

/**
 replace an existing module in a compiler

 all imports will be recalculated
 */
let update_module = (id: Namespace.t, compiler: t) => {
  let (removed, _) as result =
    compiler.graph |> ImportGraph.refresh_subtree(id);

  removed |> List.iter(id => compiler.modules |> ModuleTable.remove(id));

  result;
};

/**
 remove a module from a compiler

 any modules which are only imported by the removed module
 will also be removed
 */
let remove_module = (id: Namespace.t, compiler: t) => {
  let removed = compiler.graph |> ImportGraph.prune_subtree(id);

  removed |> List.iter(id => compiler.modules |> ModuleTable.remove(id));

  removed;
};

/**
 move a module to a new location
 */
let relocate_module = (id: Namespace.t, compiler: t) =>
  resolve(~skip_cache=true, compiler, id) |> Module.exists
    ? update_module(id, compiler)
    : remove_module(id, compiler) |> (removed => (removed, []));

/**
 destroy any resources reserved by the compiler
 */

let teardown = (compiler: t) => compiler.resolver.cache |> Cache.destroy;
