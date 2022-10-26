open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let multiply = (ctx: ParseContext.t) =>
  AST.Raw.of_mult_op |> Util.binary_op(ctx) <$ Symbol.multiply;
