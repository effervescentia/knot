open Knot.Kore;
open Parse.Onyx;

let parse = Parser.unequal;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.inequality);
