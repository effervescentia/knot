open Kore;

/**
 resolves modules on disk based on an identifier
 */
type t = {
  root_dir: string,
  source_dir: string,
  cache: Cache.t,
};

let _get_source_path = name => name ++ Constants.file_extension;

let _to_module = (path, resolver) =>
  Filename.concat(resolver.source_dir, path)
  |> (
    relative =>
      Module.of_file({
        full: Filename.concat(resolver.root_dir, relative),
        relative,
      })
  );

/* static */

let create = (cache: Cache.t, root_dir: string, source_dir: string): t => {
  root_dir,
  source_dir,
  cache,
};

/* methods */

/**
 find a file either in the cache or source directories
 */
let resolve_module = (~skip_cache=false, id: m_id, resolver: t): Module.t =>
  switch (id, skip_cache) {
  /* resolve from cache if not skipping cache */
  | (Internal(name), false) =>
    let path = _get_source_path(name);

    resolver.cache
    |> Cache.resolve_path(path)
    |> (
      full =>
        Module.of_file({
          full,
          relative: Filename.concat(resolver.source_dir, path),
        })
    );
  /* resolve from source directory if skipping cache */
  | (Internal(name), true) =>
    Filename.concat(resolver.source_dir, _get_source_path(name))
    |> (
      relative =>
        Module.of_file({
          full: Filename.concat(resolver.root_dir, relative),
          relative,
        })
    )
  /* resolve an external module */
  | (External(path), _) => raise(NotImplemented)
  };
