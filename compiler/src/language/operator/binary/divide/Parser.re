open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let divide: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_div_op) <$ Symbol.divide;
