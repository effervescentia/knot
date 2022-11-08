open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let less_than: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_lt_op)
  <$ Matchers.symbol(Constants.Character.open_chevron);
