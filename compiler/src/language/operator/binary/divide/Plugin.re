open Knot.Kore;

let parse = Parser.divide;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "/");
