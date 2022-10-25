open Knot.Kore;

let parse = Parser.add;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "+");
