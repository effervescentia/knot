open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;

let logical_or = (ctx: ParseContext.t) =>
  AST.Raw.of_or_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.logical_or;
