open Kore;
open Resolve.Kore;

module Cache = File.Cache;
module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module ImportGraph = Resolve.ImportGraph;
module Parser = Parse.Parser;
module Program = Grammar.Program;

exception FailedToParse;

type static_config = {
  name: string,
  root_dir: string,
  entry: m_id,
};

let _parse_imports =
  AST.(
    input =>
      Parser.parse(Program.imports, input)
      |> (
        fun
        | Some(stmts) =>
          stmts
          |> List.filter_map(
               fun
               | Import(m_id, _) => Some(to_m_id(m_id))
               | _ => None,
             )
        | None => raise(FailedToParse)
      )
  );

let _parse_ast =
  AST.(
    input =>
      Parser.parse(Program.main, input)
      |> (
        fun
        | Some(stmts) => stmts
        | None => raise(FailedToParse)
      )
  );

let run = (cfg: static_config) => {
  let cache = Cache.create(cfg.name);

  /* resolve files from the root directory */
  let root_dir_resolver = Resolver.create(cfg.root_dir);

  /* teardown on SIGTERM */
  Sys.set_signal(Sys.sigterm, Sys.Signal_handle(_ => Cache.destroy(cache)));

  /* generate import graph */
  let graph =
    ImportGraph.create(
      cfg.entry,
      Resolver.resolve_module(root_dir_resolver)
      % Module.read(_parse_imports),
    );

  ImportGraph.to_string(graph) |> Log.info("%s");

  /* find import cycles */
  let cycles =
    ImportGraph.find_cycles(graph) |> (List.map(print_m_id) |> List.map);

  if (List.length(cycles) != 0) {
    raise(CyclicImports(cycles));
  };

  let modules = ImportGraph.get_modules(graph);

  /* cache snapshot of modules on disk */
  modules
  |> List.iter(
       Resolver.resolve_module(root_dir_resolver) % Module.cache(cache),
     );

  /* resolve files from the cache directory */
  let cache_resolver = Resolver.create(~cache, cfg.root_dir);

  /* table for type lookups */
  let module_types =
    Hashtbl.create(graph |> ImportGraph.get_modules |> List.length);

  /* parse modules to AST */
  modules
  |> List.iter(id =>
       Resolver.resolve_module(cache_resolver, id)
       |> Module.read(_parse_ast)
       |> (
         ast => {
           ast
           |> List.iter(
                fun
                | AST.Declaration(name, _) =>
                  Hashtbl.add(module_types, (id, name), AST.K_Invalid)
                | _ => (),
              );

           switch (id) {
           | Internal(path) =>
             Print.many(~separator="\n", AST.print_mod_stmt, ast)
             |> Log.debug("\n/* %s */\n\n%s", path)
           | _ => assert(false)
           };
         }
       )
     );

  /* clean up cache directory */
  Cache.destroy(cache);
};
