open Knot.Kore;
open Parse.Onyx;

let parse = Parser.negative;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.minus_sign);
