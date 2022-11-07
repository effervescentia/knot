open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let absolute: unary_op_parser_t =
  AST.Raw.of_pos_op |> Util.unary_op <$ Symbol.positive;
