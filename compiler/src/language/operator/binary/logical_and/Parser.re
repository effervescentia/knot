open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;

let logical_and = (ctx: ParseContext.t) =>
  AST.Raw.of_and_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.logical_and;
