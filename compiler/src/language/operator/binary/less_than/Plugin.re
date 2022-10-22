open Knot.Kore;
open Parse.Onyx;

let parse = Parser.less_than;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "<");
