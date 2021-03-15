open Kore;

type config_t = {
  name: string,
  entry: namespace_t,
  root_dir: string,
  source_dir: string,
};

/**
 parses and transforms programs represented as a graph of modules
 */
type t = {
  config: config_t,
  graph: ImportGraph.t,
  modules: ModuleTable.t,
  resolver: Resolver.t,
  throw: list(compiler_err) => unit,
  errors: ref(list(compiler_err)),
};

let __module_table_size = 64;

let _resolve = (~skip_cache=false, compiler: t, id: namespace_t) =>
  Resolver.resolve_module(~skip_cache, id, compiler.resolver);

let _add_to_cache = (id: namespace_t, compiler: t) =>
  _resolve(~skip_cache=true, compiler, id)
  |> Module.cache(compiler.resolver.cache);

let _get_exports = (ast: AST.program_t) =>
  ast
  |> List.filter_map(
       fun
       | AST.Declaration(Private(_), _) => None
       | AST.Declaration(Public(name), _) => Some((name, Type.K_Invalid))
       | _ => None,
     );

let _report_errors = (compiler: t) =>
  if (List.length(compiler.errors^) != 0) {
    let errors = compiler.errors^;

    compiler.errors := [];
    compiler.throw(errors);
  };

let _add_errors =
    (errors: list(compiler_err), errors_ref: ref(list(compiler_err))) => {
  errors_ref := errors @ errors_ref^;
};

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
let create = (~catch as throw=throw_all, config: config_t): t => {
  let cache = Cache.create(config.name);
  let resolver = Resolver.create(cache, config.root_dir, config.source_dir);

  let errors = ref([]);
  let graph =
    ImportGraph.create(id =>
      try(
        resolver
        |> Resolver.resolve_module(~skip_cache=true, id)
        |> Module.read(Parser.imports)
      ) {
      | CompilerError(e) =>
        _add_errors(e, errors);
        [];
      }
    );

  let modules = Hashtbl.create(__module_table_size);

  {throw, config, graph, modules, resolver, errors};
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
let process =
    (ids: list(namespace_t), resolve: namespace_t => Module.t, compiler: t) => {
  ids
  |> List.iter(id =>
       try(
         resolve(id)
         |> Module.read(Parser.ast)
         |> (
           ast =>
             compiler.modules |> ModuleTable.add(id, ast, _get_exports(ast))
         )
       ) {
       | CompilerError(e) => _add_errors(e, compiler.errors)
       }
     );

  compiler |> _report_errors;
};

/**
 fill import graph from entry and parse program to AST
 */
let init = (~skip_cache=false, compiler: t) => {
  compiler.graph |> ImportGraph.init(compiler.config.entry);
  compiler |> _report_errors;

  /* check if import graph is valid */
  compiler |> validate;

  let modules = ImportGraph.get_modules(compiler.graph);

  if (!skip_cache) {
    /* cache snapshot of modules on disk */
    modules |> List.iter(id => _add_to_cache(id, compiler));
  };

  /* parse modules to AST */
  compiler |> process(modules, _resolve(~skip_cache, compiler));
};

/**
 re-evaluate a subset of the import graph
 */
let incremental = (ids: list(namespace_t), compiler: t) => {
  compiler |> validate;
  compiler |> process(ids, _resolve(~skip_cache=true, compiler));

  /* generate output files */

  Log.info("incremental compilation successful");
};

/**
 use the ASTs of parsed modules to generate files in the target format
 */
let emit_output = (target: Target.t, output_dir: string, compiler: t) => {
  compiler.modules
  |> Hashtbl.iter(
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
                 {
                   print: Printf.fprintf(out, "%s"),
                   resolve:
                     fun
                     | Internal(path) =>
                       Filename.concat(output_dir, path)
                       |> Filename.relative_to(parent_dir)
                     | External(_) => raise(NotImplemented),
                 },
                 ast,
               );
               close_out(out);
             };
           }
         )
       | External(_) => raise(NotImplemented),
     );
};

/**
 compile the entire program to the target
 */
let compile = (target: Target.t, output_dir: string, compiler: t) => {
  compiler |> init;

  FileUtil.rm(~recurse=true, [output_dir]);
  FileUtil.mkdir(~parent=true, output_dir);

  compiler |> emit_output(target, output_dir);

  /* generate output files */

  Log.info("compilation successful");
};

/**
 add a new module (and its import graph) to a compiler
 */
let add_module = (id: namespace_t, compiler: t) =>
  compiler.graph |> ImportGraph.add_module(id);

/**
 replace an existing module in a compiler

 all imports will be recalculated
 */
let update_module = (id: namespace_t, compiler: t) => {
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
let remove_module = (id: namespace_t, compiler: t) => {
  let removed = compiler.graph |> ImportGraph.prune_subtree(id);

  removed |> List.iter(id => compiler.modules |> ModuleTable.remove(id));

  removed;
};

/**
 move a module to a new location
 */
let relocate_module = (id: namespace_t, compiler: t) =>
  _resolve(~skip_cache=true, compiler, id) |> Module.exists
    ? update_module(id, compiler)
    : remove_module(id, compiler) |> (removed => (removed, []));

/**
 destroy any resources reserved by the compiler
 */

let teardown = (compiler: t) => compiler.resolver.cache |> Cache.destroy;
