open Knot.Kore;

let parse = Parser.parse;

let format: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.greater_or_eql);
