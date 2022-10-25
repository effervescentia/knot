open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let greater_than = (ctx: ParseContext.t) =>
  AST.Raw.of_gt_op
  |> Util.binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Symbol.greater_than;
