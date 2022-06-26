open Infix;

module Namespace = {
  type t =
    | Internal(string)
    | External(string)
    | Ambient
    | Stdlib;

  exception CannotExtractPath;

  /* static */

  let of_internal = value => Internal(value);

  let of_string = value =>
    value |> String.starts_with(Constants.root_dir)
      ? value |> String.drop_prefix(Constants.root_dir) |> of_internal
      : External(value);

  let of_path = value =>
    value |> String.drop_suffix(Constants.file_extension) |> of_internal;

  /* methods */

  let to_path = (~ext=Constants.file_extension, parent_dir: string) =>
    fun
    | Internal(path) => Filename.concat(parent_dir, path ++ ext)
    | External(path) => path
    | Ambient
    | Stdlib => raise(CannotExtractPath);

  let to_string =
    fun
    | Internal(path) => Constants.root_dir ++ path
    | External(path) => path
    | Ambient => "ambient"
    | Stdlib => "stdlib";

  /* pretty printing */

  let pp: Fmt.t(t) = ppf => to_string % Fmt.string(ppf);
};

module Export = {
  type t =
    | Main
    | Named(string);

  /* methods */

  let to_string =
    fun
    | Main => "main"
    | Named(name) => name;

  /* pretty printing */

  let pp: Fmt.t(t) = ppf => to_string % Fmt.string(ppf);
};
