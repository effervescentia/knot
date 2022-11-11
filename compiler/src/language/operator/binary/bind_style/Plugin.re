open Knot.Kore;

let parse = Parser.bind_style;

let pp: Fmt.t(unit) =
  (ppf, ()) => Fmt.string(ppf, Constants.Glyph.style_binding);
