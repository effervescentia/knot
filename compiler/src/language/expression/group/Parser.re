open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Symbol = Parse.Symbol;

let group =
    (parse_expr: Parse.Kore.expression_parser_t)
    : Parse.Kore.expression_parser_t =>
  Matchers.between(Symbol.open_group, Symbol.close_group, parse_expr)
  >|= (
    ((expr, _) as expr_node) =>
      Node.typed(
        AST.Raw.of_group(expr),
        Node.get_type(expr),
        Node.get_range(expr_node),
      )
  );
