open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let subtract = (ctx: ParseContext.t) =>
  AST.Raw.of_sub_op
  |> Util.binary_op(ctx, ~get_type=Util.get_arithmetic_result_type)
  <$ Symbol.subtract;
