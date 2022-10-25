open Knot.Kore;

let parse = Parser.expo;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "^");
