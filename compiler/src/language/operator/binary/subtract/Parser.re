open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let subtract = (ctx: ParseContext.t) =>
  AST.Raw.of_sub_op |> Util.binary_op(ctx) <$ Symbol.subtract;
