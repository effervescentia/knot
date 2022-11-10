open Knot.Kore;

include AST.Framework.Primitive({
  type value_t = string;

  let parse = Parse.Kore.(Matchers.string >|= Node.map(AST.Raw.of_string));

  let pp = (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

  let to_xml = x => Fmt.Node("String", [("value", x |> ~@pp)], []);
});
