open Knot.Kore;

let parse = Parser.logical_and;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "&&");
