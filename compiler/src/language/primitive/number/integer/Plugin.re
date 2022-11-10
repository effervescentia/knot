open Knot.Kore;

include AST.Framework.Number({
  type value_t = int64;

  let parse = Parser.integer;

  let pp = Fmt.int64;

  let to_xml = x => Fmt.Node("Integer", [("value", x |> ~@pp)], []);
});
