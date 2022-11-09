open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let add: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_add_op)
  <$ Matchers.symbol(Constants.Character.plus_sign);
