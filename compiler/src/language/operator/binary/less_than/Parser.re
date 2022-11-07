open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let less_than: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_lt_op) <$ Symbol.less_than;
