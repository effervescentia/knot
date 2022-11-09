open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let multiply: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_mult_op)
  <$ Matchers.symbol(Constants.Character.asterisk);
