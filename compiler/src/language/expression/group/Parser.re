open Knot.Kore;
open Parse.Kore;
open AST;

let group =
    (parse_expr: ParserTypes.expression_parser_t)
    : ParserTypes.expression_parser_t =>
  parse_expr
  |> Matchers.between_parentheses
  >|= (
    ((expr, _) as expr_node) =>
      Node.typed(
        Raw.of_group(expr),
        Node.get_type(expr),
        Node.get_range(expr_node),
      )
  );
