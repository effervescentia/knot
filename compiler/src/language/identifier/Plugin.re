open Knot.Kore;

let parse = Parser.identifier;

let analyze = Analyzer.analyze_identifier;

let pp: Fmt.t(string) = Fmt.string;

let to_xml: string => Fmt.xml_t(string) =
  name => Node("Identifier", [("name", name |> ~@pp)], []);
