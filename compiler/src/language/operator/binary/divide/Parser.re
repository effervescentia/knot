open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let divide = (ctx: ParseContext.t) =>
  AST.Raw.of_div_op |> Util.binary_op(ctx) <$ Symbol.divide;
