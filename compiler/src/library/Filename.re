include Stdlib.Filename;

let normalize = (path: string) =>
  is_relative(path)
    ? concat(Sys.getcwd(), String.drop_prefix("./", path)) : path;

let relative_to = (root: string, path: string) =>
  !is_relative(path) && String.starts_with(root, path)
    ? String.drop_prefix(root, path) |> String.drop_prefix("/") : path;
