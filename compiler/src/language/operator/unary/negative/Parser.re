open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module Util = Parse.Util;

let negative: unary_op_parser_t =
  Util.unary_op(AST.Raw.of_neg_op)
  <$ Matchers.symbol(Constants.Character.minus_sign);
