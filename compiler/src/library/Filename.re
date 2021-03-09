/**
 Extension of the standard Filename module with additional functionality.
 */
include Stdlib.Filename;

exception AbsolutePathConcatentation;

let __relative_prefix = current_dir_name ++ dir_sep;
let __dir_sep_char = List.nth(dir_sep |> String.to_list, 0);

/**
 * simplify a path to its most basic form
 */
let normalize = path =>
  (String.starts_with(dir_sep, path) ? dir_sep : "")
  ++ (
    String.split_on_char(__dir_sep_char, path)
    |> List.filter(value => value != "" && value != current_dir_name)
    |> String.join(~separator=dir_sep)
  );

/**
 * concatenate two paths
 */
let concat = (l: string, r: string) =>
  is_relative(r)
    ? switch (normalize(l), normalize(r)) {
      | (l', "") => l'
      | (l', r') => concat(l', r')
      }
    : raise(AbsolutePathConcatentation);

/**
 * resolve the absolute form of a path
 */
let resolve = (path: string) =>
  if (is_relative(path)) {
    concat(Sys.getcwd(), path);
  } else {
    normalize(path);
  };
