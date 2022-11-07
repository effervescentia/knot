open Knot.Kore;

let parse = Parser.integer;

let pp: Fmt.t(int64) = Fmt.int64;

let to_xml: int64 => Fmt.xml_t(string) =
  x => Node("Integer", [("value", x |> ~@pp)], []);
