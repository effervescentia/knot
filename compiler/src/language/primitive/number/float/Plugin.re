open Knot.Kore;

let pp = (ppf, (float, precision)) => Fmt.pf(ppf, "%.*f", precision, float);

include AST.Framework.Number({
  type value_t = (float, int);

  let parse = Parser.float;

  let format = pp;

  let to_xml = x => Fmt.Node("Float", [("value", x |> ~@pp)], []);
});
