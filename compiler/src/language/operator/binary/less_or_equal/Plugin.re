open Knot.Kore;

let parse = Parser.less_or_equal;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "<=");
