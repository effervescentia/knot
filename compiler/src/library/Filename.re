include Stdlib.Filename;

let normalize = (path: string) =>
  is_relative(path)
    ? concat(
        Unix.getcwd(),
        String.starts_with("./", path) ? String.drop_left(2, path) : path,
      )
    : path;
