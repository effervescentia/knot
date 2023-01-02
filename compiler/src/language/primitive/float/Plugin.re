open Knot.Kore;

include AST.Framework.Primitive({
  type value_t = (float, int);

  let parse = Parser.parse;

  let format = (ppf, (float, precision)) =>
    Fmt.pf(ppf, "%.*f", precision, float);

  let to_xml = x => Fmt.Node("Float", [("value", x |> ~@format)], []);
});
