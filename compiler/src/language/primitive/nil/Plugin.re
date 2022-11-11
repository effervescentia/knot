open Knot.Kore;

module Keyword = Constants.Keyword;

let pp = (ppf, ()) => Keyword.nil |> Fmt.string(ppf);

include AST.Framework.Primitive({
  type value_t = unit;

  let parse = Parse.Kore.(AST.Raw.nil <$| Matchers.keyword(Keyword.nil));

  let format = pp;

  let to_xml = () => Fmt.Node("Nil", [], []);
});
