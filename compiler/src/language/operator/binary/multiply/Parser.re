open Knot.Kore;
open Parse.Kore;
open AST;

let multiply: ParserTypes.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_mult_op)
  <$ Matchers.symbol(Constants.Character.asterisk);
