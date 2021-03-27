/**
 Utilities to enable writing to and reading from a cached version of the source directory.
 */

type t = string;

/* static */

let create = (project_name: string): t => Util.create_temp_dir(project_name);

/* methods */

/**
 get the cached path of a file
 */
let resolve_path = (path: string, cache: t): string =>
  Filename.concat(cache, path);

/**
 check if a file exists within the cache
 */
let file_exists = (path: string, cache: t): bool =>
  resolve_path(path, cache) |> Sys.file_exists;

/**
 open a cached file
 */
let open_file = (path: string, cache: t): in_channel =>
  resolve_path(path, cache) |> open_in;

/**
 remove the cache directory
 */
let destroy = (temp_dir: t) => FileUtil.rm(~recurse=true, [temp_dir]);
