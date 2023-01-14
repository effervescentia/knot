open Knot.Kore;

module Keyword = Constants.Keyword;

include AST.Framework.Primitive.Make({
  type value_t = bool;

  let parse = Parser.parse;

  let format = (ppf, x) =>
    (x ? Keyword.true_ : Keyword.false_) |> Fmt.string(ppf);

  let to_xml = x => Fmt.Node("Boolean", [("value", x |> ~@format)], []);
});
