open Globals;

let mkdir_p = Core.Unix.mkdir_p;

let is_source_module = path =>
  String.length(path) > 1 && path.[0] == '.' && path.[1] != '/';

let relative_path = (prefix, path) => {
  let prefix_length = String.length(prefix);

  String.sub(
    path,
    prefix_length + 1,
    String.length(path) - prefix_length - 1,
  );
};

let to_path_segment = path =>
  Str.global_replace(
    Str.regexp("\\."),
    Filename.dir_sep,
    String.sub(path, 1, String.length(path) - 1),
  );

let normalize_module = (source_dir, path) =>
  if (String.length(path) == 0) {
    invariant(EmptyModulePath);
  } else if (is_source_module(path)) {
    to_path_segment(path) |> Filename.concat(source_dir);
  } else {
    path;
  };

let normalize_path = (root_dir, file) =>
  if (Filename.is_relative(file)) {
    Filename.concat(root_dir, file);
  } else {
    file;
  };

let is_within_dir = (parent_dir, path) =>
  String.sub(path, 0, String.length(parent_dir)) == parent_dir;

let rec clean_directory =
  fun
  | res when Sys.file_exists(res) && Sys.is_directory(res) => {
      Sys.readdir(res) |> Array.iter(Filename.concat(res) % clean_directory);

      Unix.rmdir(res);
    }

  | res => Unix.unlink(res);
