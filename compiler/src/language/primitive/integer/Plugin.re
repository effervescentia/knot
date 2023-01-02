open Knot.Kore;

include AST.Framework.Primitive({
  type value_t = int64;

  let parse = Parser.parse;

  let format = Fmt.int64;

  let to_xml = x => Fmt.Node("Integer", [("value", x |> ~@format)], []);
});
