open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let greater_than: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_gt_op) <$ Symbol.greater_than;
