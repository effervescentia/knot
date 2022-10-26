open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let less_than = (ctx: ParseContext.t) =>
  AST.Raw.of_lt_op |> Util.binary_op(ctx) <$ Symbol.less_than;
