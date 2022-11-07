open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let negative: unary_op_parser_t =
  Util.unary_op(AST.Raw.of_neg_op) <$ Symbol.negative;
