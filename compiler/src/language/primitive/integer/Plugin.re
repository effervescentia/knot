open Knot.Kore;

include AST.Framework.Primitive.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let format = Fmt.int64;

  let to_xml = x => Fmt.Node("Integer", [("value", x |> ~@format)], []);
});
