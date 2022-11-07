open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let less_or_equal: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_lte_op) <$ Matchers.glyph("<=");
