open Knot.Kore;

include AST.Framework.Primitive.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let format = (ppf, x) =>
    (x ? Constants.Keyword.true_ : Constants.Keyword.false_)
    |> Fmt.string(ppf);

  let to_xml = x => Fmt.Node("Boolean", [("value", x |> ~@format)], []);
});
