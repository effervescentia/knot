open Knot.Kore;

module Keyword = Constants.Keyword;

let pp = (ppf, x) => (x ? Keyword.true_ : Keyword.false_) |> Fmt.string(ppf);

include AST.Framework.Primitive({
  type value_t = bool;

  let parse = Parser.boolean;

  let format = pp;

  let to_xml = x => Fmt.Node("Boolean", [("value", x |> ~@pp)], []);
});
