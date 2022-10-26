open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let logical_not = (ctx: ParseContext.t) =>
  AST.Raw.of_not_op |> Util.unary_op(ctx) <$ Symbol.not;
