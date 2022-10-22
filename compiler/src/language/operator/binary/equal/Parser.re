open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Util = Grammar.Util;
let equal = (ctx: ParseContext.t) =>
  AST.Raw.of_eq_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.equality;
