open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;

let unequal = (ctx: ParseContext.t) =>
  AST.Raw.of_ineq_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.inequality;
