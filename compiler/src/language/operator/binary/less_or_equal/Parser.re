open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;

let less_or_equal = (ctx: ParseContext.t) =>
  AST.Raw.of_lte_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.less_or_eql;
