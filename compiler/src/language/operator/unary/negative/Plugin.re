open Knot.Kore;

let parse = Parser.parse;

let format: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.minus_sign);
