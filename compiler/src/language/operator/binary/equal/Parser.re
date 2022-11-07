open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let equal: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_eq_op) <$ Matchers.glyph("==");
