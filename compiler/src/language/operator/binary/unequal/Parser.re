open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let unequal: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_ineq_op) <$ Matchers.glyph("!=");
