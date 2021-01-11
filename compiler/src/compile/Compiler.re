open Kore;

type config_t = {
  name: string,
  entry: m_id,
  root_dir: string,
};

type t = {
  config: config_t,
  cache: Cache.t,
  graph: ImportGraph.t,
  modules: ModuleTable.t,
  resolve_from_source: m_id => Module.t,
  resolve_from_cache: m_id => Module.t,
  catch: compiler_err => unit,
};

let _add_to_cache = (id: m_id, compiler: t) =>
  compiler.resolve_from_source(id) |> Module.cache(compiler.cache);

let _remove_module_types = (id: m_id, compiler: t) =>
  compiler.modules |> ModuleTable.remove(id);

let _get_exports = (ast: AST.program_t) =>
  ast
  |> List.filter_map(
       fun
       | AST.Declaration(name, _) when name |> String.starts_with("_") =>
         None
       | AST.Declaration(name, _) => Some((name, Type.K_Invalid))
       | _ => None,
     );

let _print_import_graph = (compiler: t) =>
  ImportGraph.to_string(compiler.graph)
  |> Log.debug("\n\n--- import graph ---\n\n%s");

let _print_modules = (compiler: t) =>
  ModuleTable.to_string(compiler.modules)
  |> Log.debug("\n\n--- modules ---\n\n%s");

let create = (~catch=throw, config: config_t): t => {
  let resolve_from_source =
    Resolver.create(config.root_dir) |> Resolver.resolve_module;

  let cache = Cache.create(config.name);
  let resolve_from_cache =
    Resolver.create(~cache, config.root_dir) |> Resolver.resolve_module;

  let errors = ref([]);
  let graph =
    ImportGraph.create(config.entry, id =>
      try(resolve_from_source(id) |> Module.read(Parser.imports)) {
      | CompilerError(err) =>
        errors := [err, ...errors^];
        [];
      }
    );

  if (List.length(errors^) != 0) {
    catch(ErrorList(errors^));
  };

  let modules =
    Hashtbl.create(graph |> ImportGraph.get_modules |> List.length);

  {
    config,
    catch,
    cache,
    graph,
    modules,
    resolve_from_source,
    resolve_from_cache,
  };
};

let validate = (compiler: t) => {
  compiler.graph
  |> ImportGraph.find_cycles
  |> (List.map(print_m_id) |> List.map)
  |> (
    fun
    | [] => ()
    | cycles => throw_all(cycles |> List.map(cycle => ImportCycle(cycle)))
  );

  compiler.graph
  |> ImportGraph.find_missing
  |> List.map(print_m_id)
  |> (
    fun
    | [] => ()
    | missing =>
      throw_all(missing |> List.map(path => UnresolvedModule(path)))
  );
};

let process = (ids: list(m_id), resolver: m_id => Module.t, compiler: t) => {
  let errors = ref([]);
  ids
  |> List.iter(id =>
       try(
         resolver(id)
         |> Module.read(Parser.ast)
         |> (
           ast =>
             compiler.modules |> ModuleTable.add(id, ast, _get_exports(ast))
         )
       ) {
       | CompilerError(err) => errors := [err, ...errors^]
       }
     );

  if (List.length(errors^) != 0) {
    compiler.catch(ErrorList(errors^));
  };
};

let initialize = (~cache=true, compiler: t) => {
  compiler |> _print_import_graph;

  /* find import cycles and missing imports */
  try(compiler |> validate) {
  | CompilerError(err) => compiler.catch(err)
  };

  let modules = ImportGraph.get_modules(compiler.graph);

  if (cache) {
    /* cache snapshot of modules on disk */
    modules |> List.iter(id => _add_to_cache(id, compiler));
  };

  /* parse modules to AST */
  compiler
  |> process(
       modules,
       cache ? compiler.resolve_from_cache : compiler.resolve_from_source,
     );
  /* compiler |> _print_modules; */
};

let incremental = (ids: list(m_id), compiler: t) => {
  compiler |> _print_import_graph;

  compiler |> validate;
  compiler |> process(ids, compiler.resolve_from_source);

  /* compiler |> _print_modules; */

  /* generate output files */
  ();

  Log.info("incremental compilation successful");
};

let compile = (compiler: t) => {
  initialize(compiler);

  /* generate output files */
  ();

  Log.info("compilation successful");
};

let add_module = (id: m_id, compiler: t) =>
  compiler.graph |> ImportGraph.add_module(id);

let update_module = (id: m_id, compiler: t) => {
  let (removed, _) as result =
    compiler.graph |> ImportGraph.refresh_subtree(id);

  removed |> List.iter(id => compiler |> _remove_module_types(id));

  result;
};

let remove_module = (id: m_id, compiler: t) => {
  let removed = compiler.graph |> ImportGraph.prune_subtree(id);

  removed |> List.iter(id => compiler |> _remove_module_types(id));

  removed;
};

let relocate_module = (id: m_id, compiler: t) =>
  compiler.resolve_from_source(id) |> Module.exists
    ? update_module(id, compiler)
    : remove_module(id, compiler) |> (removed => (removed, []));

let teardown = (compiler: t) => compiler.cache |> Cache.destroy;
