open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let logical_or: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_or_op) <$ Matchers.glyph("||");
