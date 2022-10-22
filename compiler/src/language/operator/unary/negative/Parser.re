open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let negative = (ctx: ParseContext.t) =>
  AST.Raw.of_neg_op |> Util.unary_op(ctx) <$ Symbol.negative;
