open Knot.Kore;

include AST.Framework.Number({
  type value_t = (float, int);

  let parse = Parser.float;

  let pp = (ppf, (float, precision)) =>
    Fmt.pf(ppf, "%.*f", precision, float);

  let to_xml = x => Fmt.Node("Float", [("value", x |> ~@pp)], []);
});
