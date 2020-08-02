open Core;

let is_source_module = path =>
  String.length(path) > 1 && path.[0] == '.' && path.[1] != '/';

let chop_path_prefix = (prefix, path) => {
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
