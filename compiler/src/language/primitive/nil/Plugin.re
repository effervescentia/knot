open Knot.Kore;

module Keyword = Constants.Keyword;

include AST.Framework.Primitive.Make({
  type value_t = unit;

  let parse = Parse.Kore.(() <$| Matchers.keyword(Keyword.nil));

  let format = (ppf, ()) => Keyword.nil |> Fmt.string(ppf);

  let to_xml = () => Fmt.Node("Nil", [], []);
});
