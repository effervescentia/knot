open Kore;

module Cache = File.Cache;
module Resolver = Resolve.Resolver;
module ImportGraph = Resolve.ImportGraph;

type static_config = {
  name: string,
  root_dir: string,
  entry: m_id,
};

let run = (cfg: static_config) => {
  let cache = Cache.create(cfg.name);
  let resolver = Resolver.create(cfg.root_dir);
  let graph = ImportGraph.create(cfg.entry);

  /*

   modules = get_import_graph;

   */

  /* teardown on SIGKILL */
  Sys.set_signal(Sys.sigkill, Sys.Signal_handle(_ => Cache.destroy(cache)));

  /* let resolver = Resolver.create(cfg.root_dir, cache);
     let module_factory = Module.create(resolver); */
  /* let modules = Resolver. */

  Cache.destroy(cache);
  ();
};
