open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let greater_than = (ctx: ParseContext.t) =>
  AST.Raw.of_gt_op |> Util.binary_op(ctx) <$ Symbol.greater_than;
