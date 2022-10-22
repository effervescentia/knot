open Knot.Kore;
open Parse.Onyx;

let parse = Parser.logical_not;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.char(ppf, Constants.Character.exclamation_mark);
