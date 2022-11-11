open Knot.Kore;
open Parse.Kore;
open AST;

module Util = Parse.Util;

let negative: Framework.unary_op_parser_t =
  Util.unary_op(Raw.of_neg_op)
  <$ Matchers.symbol(Constants.Character.minus_sign);
