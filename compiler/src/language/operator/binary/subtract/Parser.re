open Knot.Kore;
open Parse.Kore;
open AST;

let subtract: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_sub_op)
  <$ Matchers.symbol(Constants.Character.minus_sign);
