open Knot.Kore;

include AST.Framework.Primitive.Make({
  include AST.Framework.Primitive.MakeTypes({
    type value_t = unit;
  });

  let parse = Parse.Kore.(() <$| Matchers.keyword(Constants.Keyword.nil));

  let format = (ppf, ()) => Constants.Keyword.nil |> Fmt.string(ppf);

  let to_xml = () => Fmt.Node("Nil", [], []);
});
