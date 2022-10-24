open Knot.Kore;

let parse = Parser.greater_than;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, ">");
