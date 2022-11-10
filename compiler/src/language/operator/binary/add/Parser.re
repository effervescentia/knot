open Knot.Kore;
open Parse.Kore;
open AST;

let add: ParserTypes.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_add_op)
  <$ Matchers.symbol(Constants.Character.plus_sign);
