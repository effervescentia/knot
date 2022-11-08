open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let subtract: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_sub_op)
  <$ Matchers.symbol(Constants.Character.minus_sign);
