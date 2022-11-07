open Knot.Kore;

let parse = Parser.boolean;

let analyze = x => Node.add_type(AST.Type.Valid(`Boolean), x);

let pp = Formatter.pp_boolean;

let to_xml: bool => Fmt.xml_t(string) =
  x => Node("Boolean", [("value", x |> ~@pp)], []);
