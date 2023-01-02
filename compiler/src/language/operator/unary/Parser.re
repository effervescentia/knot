open Kore;
open Parse.Kore;
open AST;

let parse =
    (parse_expr: Framework.expression_parser_t): Framework.expression_parser_t =>
  choice([KLogicalNot.parse, KAbsolute.parse, KNegative.parse])
  |> Matchers.unary_op(parse_expr);
