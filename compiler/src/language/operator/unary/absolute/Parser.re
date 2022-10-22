open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let absolute = (ctx: ParseContext.t) =>
  AST.Raw.of_pos_op |> Util.unary_op(ctx) <$ Symbol.positive;
