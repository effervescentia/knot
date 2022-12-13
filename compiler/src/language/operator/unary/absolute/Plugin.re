open Knot.Kore;

let parse = Parser.parse;

let format: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.plus_sign);
