open Knot.Kore;

let parse = Parser.unequal;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "!=");
