open Kore;

module Namespace = Reference.Namespace;
module Export = Reference.Export;

type config_t = {
  name: string,
  root_dir: string,
  source_dir: string,
  fail_fast: bool,
  log_imports: bool,
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

let _create_dispatch = (config, report) => {
  let errors = ref([]);
  let dispatch =
    fun
    | Report(errs) =>
      if (config.fail_fast) {
        report(errs);
      } else {
        errors := errors^ @ errs;
      }
    | Flush =>
      switch (errors^) {
      | [] => ()
      | errs =>
        errors := [];
        report(errs);
      };

  (errors, dispatch);
};

let _prepare_modules = (modules: list(Namespace.t), compiler: t) =>
  modules |> List.iter(id => compiler.modules |> ModuleTable.prepare(id));
let _purge_modules = (modules: list(Namespace.t), compiler: t) =>
  modules |> List.iter(id => compiler.modules |> ModuleTable.prepare(id));

/* static */

/**
 construct a new compiler instance
 */
let create = (~report=_ => throw_all, config: config_t): t => {
  let cache = Cache.create(config.name);
  let resolver = Resolver.create(cache, config.root_dir, config.source_dir);

  let (errors, dispatch) = _create_dispatch(config, report(resolver));

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

let cache_modules = (modules, compiler) => {
  Log.debug("%s", "caching modules" |> ~@Fmt.yellow_str);

  /* cache snapshot of modules on disk */
  modules
  |> List.iter(id =>
       switch (
         resolve(~skip_cache=true, compiler, id)
         |> Module.cache(compiler.resolver.cache)
       ) {
       | Ok(path) =>
         Log.debug(
           "cached module %s",
           (Namespace.to_string(id), path) |> ~@Fmt.captioned,
         )
       | Error(module_errors) => Report(module_errors) |> compiler.dispatch
       }
     );
};

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
  let module_errors = ref([]);

  let context =
    NamespaceContext.create(
      ~modules=compiler.modules,
      ~report=
        err => {
          module_errors := module_errors^ @ [err];

          Report([err]) |> compiler.dispatch;
        },
      namespace,
    );

  module_
  |> Module.read_to_string
  |> (
    fun
    | Ok(raw) =>
      raw
      |> File.IO.read_string
      |> Parser.ast(context)
      |> (
        fun
        | Ok(ast) => {
            let data =
              ModuleTable.{
                ast,
                raw,
                exports: _get_exports(ast) |> List.to_seq |> Hashtbl.of_seq,
                scopes: ScopeTree.of_context(context),
              };

            if (List.is_empty(module_errors^)) {
              compiler.modules |> ModuleTable.add(namespace, Valid(data));

              Log.debug(
                "processed module %s",
                namespace |> ~@Fmt.bold(Namespace.pp),
              );
            } else {
              compiler.modules
              |> ModuleTable.add(namespace, Invalid(data, module_errors^));

              Log.debug(
                "processed module %s with %s",
                namespace |> ~@Fmt.bold(Namespace.pp),
                List.length(module_errors^)
                |> ~@Fmt.(red(ppf => pf(ppf, "%a error(s)", bold(int)))),
              );
            };
          }

        | Error(err) => {
            Report([err]) |> compiler.dispatch;

            Log.debug(
              "failed to process module %s",
              namespace |> ~@Fmt.bad(Namespace.pp),
            );
          }
      )

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
  Log.debug("%s", "processing modules" |> ~@Fmt.yellow_str);

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

  if (compiler.config.log_imports) {
    Log.info("imports:\n%s", compiler.graph |> ~@ImportGraph.pp);
  };

  /* check if import graph is valid */
  compiler |> validate;

  let modules = ImportGraph.get_modules(compiler.graph);
  compiler |> _prepare_modules(modules);

  if (!skip_cache) {
    compiler |> cache_modules(modules);
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
};

/**
 use the ASTs of a single module to generate a file in the target format
 */
let emit_one =
    (
      target: Target.t,
      output_dir: string,
      namespace: Namespace.t,
      compiler: t,
    ) =>
  switch (namespace) {
  | Internal(_) =>
    namespace
    |> Namespace.to_path(~ext=Target.extension_of(target), output_dir)
    |> (
      path => {
        let parent_dir = Filename.dirname(path);

        parent_dir |> FileUtil.mkdir(~parent=true);

        let out = open_out(path);

        Log.debug(
          "writing module %s",
          path |> ~@Fmt.relative_path(output_dir),
        );

        ModuleTable.(
          fun
          | Valid({ast})
          | Invalid({ast}, _) => {
              Writer.write(out, ppf =>
                Generator.pp(
                  target,
                  fun
                  | Internal(path) =>
                    Filename.concat(output_dir, path)
                    |> Filename.relative_to(parent_dir)
                  | External(_) => raise(NotImplemented),
                  ppf,
                  ast,
                )
              );

              close_out(out);
            }

          | _ => ()
        );
      }
    )
  | External(_) => raise(NotImplemented)
  };

/**
 use the ASTs of parsed modules to generate files in the target format
 */
let emit = (target: Target.t, output_dir: string, compiler: t) =>
  compiler.modules
  |> Hashtbl.iter(namespace =>
       compiler |> emit_one(target, output_dir, namespace)
     );

/**
 compile the entire program to the target
 */
let compile =
    (
      ~skip_cache=false,
      target: Target.t,
      output_dir: string,
      entry: Namespace.t,
      compiler: t,
    ) => {
  compiler |> init(~skip_cache, entry);

  File.IO.purge(output_dir);

  compiler |> emit(target, output_dir);
};

/**
 get a module by its namespace
 */
let get_module = (id: Namespace.t, compiler: t) =>
  compiler.modules |> ModuleTable.find(id);

/**
 add a new module (and its import graph) to a compiler
 */
let add_module = (id: Namespace.t, compiler: t) => {
  let modules =
    if (compiler.graph |> ImportGraph.has_module(id)) {
      compiler.graph |> ImportGraph.get_imported_by(id);
    } else {
      compiler.graph |> ImportGraph.add_module(id);
    };

  compiler |> _prepare_modules(modules);

  modules;
};

/**
 replace an existing module in a compiler

 all imports will be recalculated
 */
let update_module = (id: Namespace.t, compiler: t) => {
  let (removed, updated) as result =
    compiler.graph |> ImportGraph.refresh_subtree(id);

  compiler |> _purge_modules(removed);
  compiler |> _prepare_modules(updated);

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
  let (removed, updated) as result =
    compiler.graph |> ImportGraph.remove_module(id);

  compiler |> _purge_modules(removed);
  compiler |> _prepare_modules(updated);

  result;
};

/**
 add or update a module by providing the raw text
 */
let insert_raw_module = (id: Namespace.t, contents: string, compiler: t) => {
  if (!(compiler.graph |> ImportGraph.has_module(id))) {
    compiler.graph.imports |> Graph.add_node(id);
  };

  let imports = contents |> File.IO.read_string |> Parser.imports(id);
  let added = ref([id]);

  imports
  |> List.iter(id =>
       compiler.graph |> ImportGraph.add_module(~added, id) |> ignore
     );

  compiler |> _prepare_modules(added^);
  compiler |> incremental(added^ |> List.excl(id));
  compiler |> process_one(id, Raw(contents));
};

/**
 reset the state of the compiler
 */
let reset = ({graph, modules}: t) => {
  ImportGraph.clear(graph);
  Hashtbl.reset(modules);
};

/**
 destroy any resources reserved by the compiler
 */
let teardown = (compiler: t) => {
  reset(compiler);
  compiler.resolver.cache |> Cache.destroy;
};
