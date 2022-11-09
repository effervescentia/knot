open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module Util = Parse.Util;

let absolute: unary_op_parser_t =
  AST.Raw.of_pos_op
  |> Util.unary_op
  <$ Matchers.symbol(Constants.Character.plus_sign);
