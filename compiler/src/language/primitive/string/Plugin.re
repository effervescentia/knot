open Knot.Kore;

include AST.Framework.Primitive({
  type value_t = string;

  let parse = Parse.Kore.Matchers.string;

  let format = (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

  let to_xml = x => Fmt.Node("String", [("value", x |> ~@format)], []);
});
