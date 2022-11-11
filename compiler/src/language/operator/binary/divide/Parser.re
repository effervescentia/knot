open Knot.Kore;
open Parse.Kore;
open AST;

let divide: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_div_op)
  <$ Matchers.symbol(Constants.Character.forward_slash);
