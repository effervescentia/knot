open Knot.Kore;

let parse = Parser.logical_or;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "||");
