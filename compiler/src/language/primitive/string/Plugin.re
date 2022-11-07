open Knot.Kore;

module Matchers = Parse.Matchers;

let parse = Parser.string;

let analyze = x => Node.add_type(AST.Type.Valid(`String), x);

let pp: Fmt.t(string) =
  (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

let to_xml: string => Fmt.xml_t(string) =
  x => Node("String", [("value", x |> ~@pp)], []);
