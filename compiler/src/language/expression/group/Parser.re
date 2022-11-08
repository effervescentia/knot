open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let group = (parse_expr: expression_parser_t): expression_parser_t =>
  parse_expr
  |> Matchers.between_parentheses
  >|= (
    ((expr, _) as expr_node) =>
      Node.typed(
        AST.Raw.of_group(expr),
        Node.get_type(expr),
        Node.get_range(expr_node),
      )
  );
