open Knot.Kore;
open Parse.Onyx;

let parse = Parser.multiply;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "*");
