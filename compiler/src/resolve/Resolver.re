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
let resolve_module =
    (~skip_cache=false, id: Reference.Namespace.t, resolver: t): Module.t =>
  switch (id, skip_cache) {
  /* resolve from cache if not skipping cache */
  | (Internal(name), false) =>
    Filename.concat(resolver.source_dir, _get_source_path(name))
    |> (
      relative =>
        resolver.cache
        |> Cache.resolve_path(relative)
        |> (full => Module.of_file({full, relative}))
    )
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

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, resolver) =>
    (
      "Resolver",
      [("cache", resolver.cache), ("root_dir", resolver.root_dir)],
    )
    |> Fmt.(struct_(string, string, ppf));
