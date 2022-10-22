open Knot.Kore;
open Parse.Onyx;

let parse = Parser.subtract;

let pp: Fmt.t(unit) = (ppf, ()) => Fmt.string(ppf, "-");
