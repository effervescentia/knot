open Knot.Kore;
open Parse.Onyx;

let parse = Parser.divide;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "/");
