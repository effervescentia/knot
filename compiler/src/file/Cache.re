/**
 * Utilities to enable writing to and reading from a cached version of the source directory.
 */

type t = string;

let __source_prefix = "source";

let create = (project_name: string): t => {
  let temp_dir = Util.create_temp_dir(project_name, __source_prefix);

  temp_dir;
};

let resolve_path = (path: string, cache: t): string =>
  Filename.concat(cache, path);

let file_exists = (path: string, cache: t): bool =>
  Filename.concat(cache, path) |> Sys.file_exists;

let open_file = (path: string, cache: t): in_channel =>
  Filename.concat(cache, path) |> open_in;

let destroy = (temp_dir: t) => Util.remove_dir(temp_dir);
