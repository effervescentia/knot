open Kore;

type config_t = {
  name: string,
  entry: m_id,
  root_dir: string,
  source_dir: string,
};

/**
 parses and transforms programs represented as a graph of modules
 */
type t = {
  config: config_t,
  cache: Cache.t,
  graph: ImportGraph.t,
  modules: ModuleTable.t,
  resolve_from_source: m_id => Module.t,
  resolve_from_cache: m_id => Module.t,
  throw: compiler_err => unit,
};

let _add_to_cache = (id: m_id, compiler: t) =>
  compiler.resolve_from_source(id) |> Module.cache(compiler.cache);

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

/* static */

/**
 construct a new compiler instance
 */
let create = (~catch as throw=throw, config: config_t): t => {
  let resolve_from_source =
    Resolver.create(config.root_dir) |> Resolver.resolve_module;

  let cache = Cache.create(config.name);
  let resolve_from_cache =
    Resolver.create(~cache, config.root_dir) |> Resolver.resolve_module;

  let errors = ref([]);
  let graph =
    ImportGraph.create(id =>
      try(resolve_from_source(id) |> Module.read(Parser.imports)) {
      | CompilerError(err) =>
        errors := [err, ...errors^];
        [];
      }
    );

  graph |> ImportGraph.init(config.entry);

  if (List.length(errors^) != 0) {
    throw(ErrorList(errors^));
  };

  let modules =
    Hashtbl.create(graph |> ImportGraph.get_modules |> List.length);

  {
    throw,
    config,
    cache,
    graph,
    modules,
    resolve_from_source,
    resolve_from_cache,
  };
};

/* methods */

/**
 check for import cycles and missing modules
 */
let validate = (compiler: t) => {
  compiler.graph |> Validate.no_import_cycles;
  compiler.graph |> Validate.no_unresolved_modules;
};

/**
 parse modules and add to table
 */
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
    compiler.throw(ErrorList(errors^));
  };
};

/**
 parse modules in the active import graph
 */
let initialize = (~cache=true, compiler: t) => {
  compiler |> _print_import_graph;

  /* check if import graph is valid */
  try(compiler |> validate) {
  | CompilerError(err) => compiler.throw(err)
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

/**
 re-evaluate a subset of the import graph
 */
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

/**
 add a new module (and its import graph) to a compiler
 */
let add_module = (id: m_id, compiler: t) =>
  compiler.graph |> ImportGraph.add_module(id);

/**
 replace an existing module in a compiler

 all imports will be recalculated
 */
let update_module = (id: m_id, compiler: t) => {
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
let remove_module = (id: m_id, compiler: t) => {
  let removed = compiler.graph |> ImportGraph.prune_subtree(id);

  removed |> List.iter(id => compiler.modules |> ModuleTable.remove(id));

  removed;
};

/**
 move a module to a new location
 */
let relocate_module = (id: m_id, compiler: t) =>
  compiler.resolve_from_source(id) |> Module.exists
    ? update_module(id, compiler)
    : remove_module(id, compiler) |> (removed => (removed, []));

/**
 destroy any resources reserved by the compiler
 */
let teardown = (compiler: t) => compiler.cache |> Cache.destroy;
