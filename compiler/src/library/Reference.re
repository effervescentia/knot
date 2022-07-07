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

module Plugin = {
  exception InvalidPlugin(string);

  let style_expression_key = "style_expression";
  let style_rule_key = "style_rule";
  let known = [style_expression_key, style_rule_key];

  type t =
    | StyleExpression
    | StyleRule;

  let of_string =
    fun
    | x when x == style_expression_key => StyleExpression
    | x when x == style_rule_key => StyleRule
    | name => raise(InvalidPlugin(name));

  let to_string =
    fun
    | StyleExpression => style_expression_key
    | StyleRule => style_rule_key;
};
