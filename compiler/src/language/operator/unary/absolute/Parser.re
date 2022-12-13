open Knot.Kore;
open Parse.Kore;
open AST;

let parse: Framework.unary_op_parser_t =
  Raw.of_pos_op
  |> Parse.Util.unary_op
  <$ Matchers.symbol(Constants.Character.plus_sign);
