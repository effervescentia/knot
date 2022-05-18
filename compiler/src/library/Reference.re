open Infix;

module Namespace = {
  type t =
    | Internal(string)
    | External(string);

  let of_string = value =>
    value |> String.starts_with(Constants.root_dir)
      ? Internal(value |> String.drop_prefix(Constants.root_dir))
      : External(value);

  let of_internal = value => Internal(value);

  /* methods */

  let to_path = (~ext=Constants.file_extension, parent_dir: string) =>
    fun
    | Internal(path) => Filename.concat(parent_dir, path ++ ext)
    | External(path) => path;

  let to_string =
    fun
    | Internal(path) => Constants.root_dir ++ path
    | External(path) => path;

  /* pretty printing */

  let pp: Fmt.t(t) = ppf => to_string % Fmt.string(ppf);
};

module Module = {
  type t =
    | Root
    | Inner(string, option(t));

  /* pretty printing */

  let rec pp: Fmt.t(t) =
    Fmt.(
      ppf =>
        fun
        | Root => string(ppf, "[root]")
        | Inner(name, None) => string(ppf, name)
        | Inner(name, Some(parent)) => pf(ppf, "%a.%s", pp, parent, name)
    );
};

module Identifier = {
  type t =
    | Private(string)
    | Public(string);

  let of_string = value =>
    value |> String.starts_with(Constants.private_prefix)
      ? Private(value |> String.drop_prefix(Constants.private_prefix))
      : Public(value);

  /* methods */

  let to_string =
    fun
    | Public(name) => name
    | Private(name) => Constants.private_prefix ++ name;

  /* pretty printing */

  let pp: Fmt.t(t) = ppf => to_string % Fmt.string(ppf);
};

module Export = {
  type t =
    | Main
    | Named(Identifier.t);

  /* methods */

  let to_string =
    fun
    | Main => "main"
    | Named(name) => Identifier.to_string(name);

  /* pretty printing */

  let pp: Fmt.t(t) = ppf => to_string % Fmt.string(ppf);
};
