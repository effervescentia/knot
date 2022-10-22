open Knot.Kore;
open Parse.Onyx;

let parse = Parser.logical_and;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.logical_and);
