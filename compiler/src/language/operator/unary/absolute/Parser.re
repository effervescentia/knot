open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let absolute: unary_op_parser_t =
  AST.Raw.of_pos_op
  |> Util.unary_op
  <$ Matchers.symbol(Constants.Character.plus_sign);
