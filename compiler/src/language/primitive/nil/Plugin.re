open Knot.Kore;

let parse = Parser.nil;

let pp: Fmt.t(unit) = (ppf, ()) => Constants.Keyword.nil |> Fmt.string(ppf);

let to_xml = (): Fmt.xml_t(string) => Node("Nil", [], []);
