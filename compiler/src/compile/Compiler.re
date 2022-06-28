open Kore;
open ModuleAliases;

module Namespace = Reference.Namespace;
module Export = Reference.Export;

type config_t = {
  name: string,
  root_dir: string,
  source_dir: string,
  fail_fast: bool,
  log_imports: bool,
  stdlib: string,
};

type action_t =
  | Fatal(list(compile_err))
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

let _create_dispatch = (config, report) => {
  let errors = ref([]);
  let dispatch =
    fun
    | Fatal(errs) => report(errs)

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
  modules |> List.iter(id => compiler.modules |> ModuleTable.purge(id));

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
        |> Source.read(Parser.imports(namespace))
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
  |> List.filter((!=)(Namespace.Stdlib))
  |> List.iter(id =>
       resolve(~skip_cache=true, compiler, id)
       |> Source.cache(compiler.resolver.cache)
       |> Result.fold(
            ~ok=
              Tuple.with_fst2(Namespace.to_string(id))
              % ~@Fmt.captioned
              % Log.debug("cached module %s"),
            ~error=module_errors =>
            Report(module_errors) |> compiler.dispatch
          )
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
       |> Result.iter_error(errs => Fatal(errs) |> compiler.dispatch)
     );

  compiler.dispatch(Flush);
};

let parse_module =
    (
      source: Source.t,
      parser: Grammar.Program.input_t => result('a, compile_err),
      compiler: t,
    ) => {
  source
  |> Source.read_to_string
  |> (
    fun
    | Ok(raw) => {
        let ast = raw |> File.IO.read_string |> parser;

        Some((raw, ast));
      }

    | Error(errs) => {
        Report(errs) |> compiler.dispatch;

        None;
      }
  );
};

/**
 parse a single module and add to table
 */
let process_one =
    (~flush=true, namespace: Namespace.t, source: Source.t, compiler: t) => {
  let module_errors = ref([]);

  let context =
    ParseContext.create(
      ~modules=compiler.modules,
      ~report=
        err => {
          module_errors := module_errors^ @ [err];

          Report([err]) |> compiler.dispatch;
        },
      namespace,
    );

  compiler
  |> parse_module(source, Parser.ast(context))
  |> Option.iter(
       fun
       | (raw, Ok(ast)) => {
           let module_ =
             ModuleTable.{
               ast,
               scopes: ScopeTree.of_context(context),
               symbols: context.symbols,
             };

           if (List.is_empty(module_errors^)) {
             compiler.modules
             |> ModuleTable.add(namespace, Valid(raw, module_));

             Log.debug(
               "processed module %s",
               namespace |> ~@Fmt.bold(Namespace.pp),
             );
           } else {
             compiler.modules
             |> ModuleTable.add(
                  namespace,
                  Partial(raw, module_, module_errors^),
                );

             Log.debug(
               "processed module %s with %s",
               namespace |> ~@Fmt.bold(Namespace.pp),
               List.length(module_errors^)
               |> ~@Fmt.(red(ppf => pf(ppf, "%a error(s)", bold(int)))),
             );
           };
         }

       | (raw, Error(err)) => {
           Report([err]) |> compiler.dispatch;

           compiler.modules
           |> ModuleTable.add(namespace, Invalid(raw, [err]));

           Log.debug(
             "failed to process module %s",
             namespace |> ~@Fmt.bad(Namespace.pp),
           );
         },
     );

  if (flush) {
    compiler.dispatch(Flush);
  };
};

/**
 parse modules and add to table
 */
let process =
    (
      ~flush=true,
      namespaces: list(Namespace.t),
      resolve: Namespace.t => Source.t,
      {modules} as compiler: t,
    ) => {
  Log.debug("%s", "processing modules" |> ~@Fmt.yellow_str);

  namespaces
  |> List.filter((!=)(Namespace.Stdlib))
  |> List.iter(namespace =>
       compiler |> process_one(~flush=false, namespace, resolve(namespace))
     );

  if (flush) {
    compiler.dispatch(Flush);
  };
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
  validate(compiler);

  let modules = ImportGraph.get_ordered_modules(compiler.graph);
  compiler |> _prepare_modules(modules);

  if (!skip_cache) {
    compiler |> cache_modules(modules);
  };

  let resolver = compiler |> resolve(~skip_cache);
  compiler |> process(modules, resolver);
};

/**
 re-evaluate a subset of the import graph
 */
let incremental = (~flush=true, ids: list(Namespace.t), compiler: t) => {
  let resolver = compiler |> resolve(~skip_cache=true);

  validate(compiler);
  compiler |> process(~flush, ids, resolver);
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

        ModuleTable.get_entry_data
        % Option.iter((ModuleTable.{ast}) => {
            Writer.write(out, ppf =>
              Generator.pp(
                target,
                fun
                | Internal(path) =>
                  Filename.concat(output_dir, path)
                  |> Filename.relative_to(parent_dir)
                | External(_)
                | Ambient
                | Stdlib => raise(NotImplemented),
                ppf,
                ast,
              )
            );

            close_out(out);
          });
      }
    )
  | External(_)
  | Ambient
  | Stdlib => raise(NotImplemented)
  };

/**
 use the ASTs of parsed modules to generate files in the target format
 */
let emit = (target: Target.t, output_dir: string, compiler: t) =>
  compiler.modules
  |> Hashtbl.iter((namespace, entry) =>
       if (namespace != Namespace.Stdlib) {
         emit_one(target, output_dir, namespace, compiler, entry);
       }
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
  let added = compiler.graph |> ImportGraph.add_module(id);
  let dependents = compiler.graph |> ImportGraph.get_dependents(id);
  let updated = added @ dependents;

  compiler |> _prepare_modules(updated);

  updated;
};

/**
 replace an existing module in a compiler

 all imports will be recalculated
 */
let update_module = (id: Namespace.t, compiler: t) => {
  compiler.graph |> ImportGraph.remove_module(id);

  compiler |> add_module(id);
};

/**
 add a new module (and its import graph) to a compiler
 */
let upsert_module = (id: Namespace.t, compiler: t) =>
  if (compiler.graph |> ImportGraph.has_module(id)) {
    compiler |> update_module(id);
  } else {
    compiler |> add_module(id);
  };

/**
 remove a module from a compiler

 any modules which are only imported by the removed module
 will also be removed
 */
let remove_module = (id: Namespace.t, compiler: t) => {
  let removed = [id];
  let updated = compiler.graph |> ImportGraph.get_dependents(id);

  compiler.graph |> ImportGraph.remove_module(id);

  compiler |> _purge_modules(removed);
  compiler |> _prepare_modules(updated);

  (removed, updated);
};

/**
 incrementally compile a module from the raw text
 */
let inject_raw = (~flush=true, id: Namespace.t, contents: string, compiler: t) => {
  if (!(compiler.graph |> ImportGraph.has_module(id))) {
    compiler.graph.imports |> Graph.add_node(id);
  };

  let added = ref([id]);
  let imports =
    contents
    |> File.IO.read_string
    |> Parser.imports(id)
    |> List.filter(x => !ImportGraph.has_module(x, compiler.graph));

  imports
  |> List.iter(child_id => {
       compiler.graph |> ImportGraph.add_module(~added, child_id) |> ignore;
       compiler.graph |> ImportGraph.add_dependency(id, child_id);
     });

  compiler |> _prepare_modules(added^);
  compiler |> incremental(~flush=false, added^ |> List.excl(id));
  compiler |> process_one(~flush, id, Raw(contents));
};

/**
 * parse and process a type definition module
 */
let process_definition =
    (~flush=true, namespace: Namespace.t, source: Source.t, compiler: t) => {
  let module_errors = ref([]);
  let ctx =
    ParseContext.create(
      ~report=
        err => {
          module_errors := module_errors^ @ [err];

          Report([err]) |> compiler.dispatch;
        },
      namespace,
    );

  let result =
    compiler
    |> parse_module(source, Parser.definition(ctx))
    |> Option.map(
         fun
         | (raw, Ok(ast)) => {
             Some((raw, ast, ctx.symbols));
           }
         | (_, Error(_)) => None,
       )
    |> Option.join;

  if (flush) {
    compiler.dispatch(Flush);
  };

  result;
};

/**
 register type definitions for the standard library
 */
let add_standard_library = (~flush=true, compiler: t) => {
  compiler.graph.imports |> Graph.add_node(Reference.Namespace.Stdlib);

  Log.info(
    "reading standard library %s",
    compiler.config.stdlib |> Fmt.str("(%s)") |> ~@Fmt.grey_str,
  );

  let namespace = Reference.Namespace.Stdlib;
  let source = Source.File({relative: "", full: compiler.config.stdlib});

  compiler
  |> process_definition(~flush=false, namespace, source)
  |> (
    fun
    | Some((raw, ast, symbols)) => {
        let library = ModuleTable.{symbols: symbols};

        compiler.modules |> ModuleTable.add(Stdlib, Library(raw, library));

        Log.debug("added standard library to compiler context");
      }
    | _ => Log.error("failed to load standard library")
  );

  if (flush) {
    compiler.dispatch(Flush);
  };
};

/**
 reset the state of the compiler
 */
let reset = ({graph, modules, resolver}: t) => {
  ImportGraph.clear(graph);
  Hashtbl.reset(modules);
  File.IO.purge(resolver.cache);
};

/**
 destroy any resources reserved by the compiler
 */
let teardown = (compiler: t) => {
  reset(compiler);
  Cache.destroy(compiler.resolver.cache);
};
