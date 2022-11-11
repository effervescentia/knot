open Knot.Kore;
open Parse.Kore;
open AST;

let logical_not: Framework.unary_op_parser_t =
  Parse.Util.unary_op(Raw.of_not_op)
  <$ Matchers.symbol(Constants.Character.exclamation_mark);
