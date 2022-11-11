open Knot.Kore;

let pp = Fmt.int64;

include AST.Framework.Number({
  type value_t = int64;

  let parse = Parser.integer;

  let format = pp;

  let to_xml = x => Fmt.Node("Integer", [("value", x |> ~@pp)], []);
});
