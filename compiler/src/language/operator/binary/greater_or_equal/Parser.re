open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let greater_or_equal = (ctx: ParseContext.t) =>
  AST.Raw.of_gte_op |> Util.binary_op(ctx) <$ Matchers.glyph(">=");
