open Knot.Kore;

let parse = Parser.multiply;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "*");
