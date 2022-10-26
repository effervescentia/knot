open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let less_or_equal = (ctx: ParseContext.t) =>
  AST.Raw.of_lte_op |> Util.binary_op(ctx) <$ Matchers.glyph("<=");
