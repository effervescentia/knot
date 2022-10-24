open Knot.Kore;

let parse = Parser.logical_not;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.exclamation_mark);
