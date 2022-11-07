open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let logical_not: unary_op_parser_t =
  Util.unary_op(AST.Raw.of_not_op) <$ Symbol.not;
