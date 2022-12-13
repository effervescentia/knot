open Knot.Kore;

module Keyword = Constants.Keyword;

include AST.Framework.Primitive({
  type value_t = unit;

  let parse = Parse.Kore.(AST.Raw.nil <$| Matchers.keyword(Keyword.nil));

  let format = (ppf, ()) => Keyword.nil |> Fmt.string(ppf);

  let to_xml = () => Fmt.Node("Nil", [], []);
});
