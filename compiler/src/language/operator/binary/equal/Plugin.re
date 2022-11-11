open Knot.Kore;

let parse = Parser.equal;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.equality);
