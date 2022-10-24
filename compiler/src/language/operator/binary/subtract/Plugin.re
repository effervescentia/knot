open Knot.Kore;

let parse = Parser.subtract;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "-");
