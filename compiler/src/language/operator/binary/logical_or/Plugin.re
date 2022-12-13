open Knot.Kore;

let parse = Parser.parse;

let format: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.logical_or);
