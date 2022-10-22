open Knot.Kore;
open Parse.Onyx;

let parse = Parser.logical_or;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.logical_or);
