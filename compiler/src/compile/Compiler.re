open Kore;

module Namespace = Reference.Namespace;
module Export = Reference.Export;

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
  |> List.map(
       Node.Raw.get_value
       % (
         fun
         /* ignore all private declarations */
         | AST.Declaration(
             MainExport((Private(_), _)) | NamedExport((Private(_), _)),
             _,
           ) =>
           []

         | AST.Declaration(NamedExport(id), decl) => [
             (Export.Named(Node.Raw.get_value(id)), Node.get_type(decl)),
           ]

         | AST.Declaration(MainExport(id), decl) => [
             (Export.Named(Node.Raw.get_value(id)), Node.get_type(decl)),
             (Export.Main, Node.get_type(decl)),
           ]

         | _ => []
       ),
     )
  |> List.flatten;

/* static */

/**
 construct a new compiler instance
 */
let create = (~report=_ => throw_all, config: config_t): t => {
  let cache = Cache.create(config.name);
  let resolver = Resolver.create(cache, config.root_dir, config.source_dir);

  let errors = ref([]);
  let dispatch =
    fun
    | Report(errs) =>
      if (config.fail_fast) {
        report(resolver, errs);
      } else {
        errors := errors^ @ errs;
      }
    | Flush =>
      switch (errors^) {
      | [] => ()
      | errs =>
        errors := [];
        report(resolver, errs);
      };

  let graph =
    ImportGraph.create(namespace =>
      switch (
        resolver
        |> Resolver.resolve_module(~skip_cache=true, namespace)
        |> Module.read(Parser.imports(namespace))
      ) {
      | Ok(x) => x
      | Error(errs)
      | exception (CompileError(errs)) =>
        Report(errs) |> dispatch;
        /* assume no imports if parsing failed */
        [];
      }
    );

  let modules = ModuleTable.create(__module_table_size);

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

let process_one = (namespace: Namespace.t, module_: Module.t, compiler: t) => {
  let context =
    NamespaceContext.create(
      ~modules=compiler.modules,
      ~report=err => Report([err]) |> compiler.dispatch,
      namespace,
    );

  module_
  |> Module.read_to_string
  |> (
    fun
    | Ok(raw) => {
        let _ast = raw |> File.IO.read_string |> Parser.ast(context);
        ();
      }
    /* TODO: uncomment */
    /* compiler.modules
       |> ModuleTable.add(
            namespace,
            ast,
            _get_exports(ast),
            ScopeTree.of_context(context),
            raw,
          ); */
    | Error(errs) => Report(errs) |> compiler.dispatch
  );
};

/**
 parse modules and add to table
 */
let process =
    (
      namespaces: list(Namespace.t),
      resolve: Namespace.t => Module.t,
      {modules} as compiler: t,
    ) => {
  namespaces
  |> List.iter(namespace =>
       process_one(namespace, resolve(namespace), compiler)
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
         | Ok(path) =>
           Log.debug(
             "successfuly cached module %s to %s",
             id |> Namespace.to_string,
             path,
           )
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
 add a new module (and its import graph) to a compiler
 */
let upsert_module = (id: Namespace.t, compiler: t) =>
  if (compiler.graph |> ImportGraph.has_module(id)) {
    compiler |> update_module(id) |> snd;
  } else {
    compiler |> add_module(id);
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
 add or update a module by providing the raw text
 */
let insert_module = (id: Namespace.t, contents: string, compiler: t) => {
  if (!(compiler.graph |> ImportGraph.has_module(id))) {
    compiler.graph.imports |> Graph.add_node(id);
  };

  let imports = contents |> File.IO.read_string |> Parser.imports(id);
  let added = ref([id]);

  imports
  |> List.iter(id =>
       ImportGraph.add_module(~added, id, compiler.graph) |> ignore
     );

  compiler |> incremental(added^ |> List.excl(id));
  compiler |> process_one(id, Raw(contents));
};

/**
 destroy any resources reserved by the compiler
 */
let teardown = (compiler: t) => compiler.resolver.cache |> Cache.destroy;
