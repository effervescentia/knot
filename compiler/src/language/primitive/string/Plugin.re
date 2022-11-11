open Knot.Kore;

let pp = (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

include AST.Framework.Primitive({
  type value_t = string;

  let parse = Parse.Kore.(Matchers.string >|= Node.map(AST.Raw.of_string));

  let format = pp;

  let to_xml = x => Fmt.Node("String", [("value", x |> ~@pp)], []);
});
