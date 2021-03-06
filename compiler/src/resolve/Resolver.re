/**
 * Resolves modules on disk based on its identifier.
 */
open Kore;

type module_t = {id: m_id};

type t = {
  root_dir: string,
  cache: option(Cache.t),
};

let create = (~cache=?, root_dir: string): t => {root_dir, cache};

let resolve_module = (resolver: t, id: m_id): Module.t =>
  switch (id, resolver.cache) {
  | (Internal(path), None) =>
    Filename.concat(resolver.root_dir, path)
    |> (full => Module.of_file({full, relative: path}))
  | (Internal(path), Some(cache)) =>
    cache
    |> Cache.resolve_path(path)
    |> (full => Module.of_file({full, relative: path}))
  | (External(path), _) => raise(NotImplemented)
  };
