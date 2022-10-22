open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;

let greater_or_equal = (ctx: ParseContext.t) =>
  AST.Raw.of_gte_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.greater_or_eql;
