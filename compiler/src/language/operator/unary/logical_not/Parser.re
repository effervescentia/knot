open Knot.Kore;
open Parse.Kore;
open AST;

let logical_not: ParserTypes.unary_op_parser_t =
  Parse.Util.unary_op(Raw.of_not_op)
  <$ Matchers.symbol(Constants.Character.exclamation_mark);
