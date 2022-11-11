open Knot.Kore;
open Parse.Kore;
open AST;

let less_than: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_lt_op)
  <$ Matchers.symbol(Constants.Character.open_chevron);
