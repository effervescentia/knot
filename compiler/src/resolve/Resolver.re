open Kore;

type module_t = {id: m_id};

/**
 resolves modules on disk based on an identifier
 */
type t = {
  root_dir: string,
  cache: Cache.t,
};

/* static */

let create = (cache: Cache.t, root_dir: string): t => {root_dir, cache};

/* methods */

/**
 find a file either in the cache or source directories
 */
let resolve_module = (~skip_cache=false, id: m_id, resolver: t): Module.t =>
  switch (id, skip_cache) {
  /* resolve from cache if not skipping cache */
  | (Internal(path), false) =>
    resolver.cache
    |> Cache.resolve_path(path)
    |> (full => Module.of_file({full, relative: path}))
  /* resolve from source directory if skipping cache */
  | (Internal(path), true) =>
    Filename.concat(resolver.root_dir, path)
    |> (full => Module.of_file({full, relative: path}))
  /* resolve an external module */
  | (External(path), _) => raise(NotImplemented)
  };
