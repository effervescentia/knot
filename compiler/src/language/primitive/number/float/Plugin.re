open Knot.Kore;

let parse = Parser.float;

let pp: Fmt.t((float, int)) =
  (ppf, (float, precision)) => Fmt.pf(ppf, "%.*f", precision, float);

let to_xml: ((float, int)) => Fmt.xml_t(string) =
  x => Node("Float", [("value", x |> ~@pp)], []);
