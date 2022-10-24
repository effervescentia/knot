open Knot.Kore;

let parse = Parser.less_than;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "<");
