open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let divide: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_div_op)
  <$ Matchers.symbol(Constants.Character.forward_slash);
