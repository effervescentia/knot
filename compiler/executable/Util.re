open Kore;

include KnotCompile.Util;

let normalize_path = (root_dir, file) =>
  if (Filename.is_relative(file)) {
    Filename.concat(root_dir, file);
  } else {
    file;
  };

let normalize_module = (source_dir, path) =>
  if (String.length(path) == 0) {
    invariant(EmptyModulePath);
  } else if (is_source_module(path)) {
    to_path_segment(path) |> Filename.concat(source_dir);
  } else {
    path;
  };

let is_within_dir = (parent_dir, path) =>
  String.sub(path, 0, String.length(parent_dir)) == parent_dir;

let rec clean_directory =
  fun
  | res when Sys.is_directory(res) => {
      Sys.readdir(res) |> Array.iter(Filename.concat(res) % clean_directory);

      Unix.rmdir(res);
    }

  | res => Unix.unlink(res);
