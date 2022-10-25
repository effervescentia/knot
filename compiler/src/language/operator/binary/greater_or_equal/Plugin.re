open Knot.Kore;

let parse = Parser.greater_or_equal;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, ">=");
