open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let greater_or_equal: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_gte_op) <$ Matchers.glyph(">=");
