open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module Util = Parse.Util;

let logical_not: unary_op_parser_t =
  Util.unary_op(AST.Raw.of_not_op)
  <$ Matchers.symbol(Constants.Character.exclamation_mark);
