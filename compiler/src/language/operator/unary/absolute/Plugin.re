open Knot.Kore;

let parse = Parser.absolute;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.plus_sign);
