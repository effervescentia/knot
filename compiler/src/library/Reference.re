module Namespace = {
  type t =
    | Internal(string)
    | External(string);

  let of_string = value =>
    value |> String.starts_with(Constants.root_dir)
      ? Internal(value |> String.drop_prefix(Constants.root_dir))
      : External(value);

  let to_string =
    fun
    | Internal(path) => Constants.root_dir ++ path
    | External(path) => path;
};

module Identifier = {
  type t =
    | Private(string)
    | Public(string);

  let of_string = value =>
    value |> String.starts_with(Constants.private_prefix)
      ? Private(value |> String.drop_prefix(Constants.private_prefix))
      : Public(value);

  let to_string =
    fun
    | Public(name) => name
    | Private(name) => Constants.private_prefix ++ name;
};

module Export = {
  type t =
    | Main
    | Named(Identifier.t);

  let to_string =
    fun
    | Main => "main"
    | Named(name) => name |> Identifier.to_string;
};
