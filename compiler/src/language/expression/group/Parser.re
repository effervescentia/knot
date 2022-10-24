open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let group =
    (parse_expr: Grammar.Kore.expression_parser_t)
    : Grammar.Kore.expression_parser_t =>
  Matchers.between(Symbol.open_group, Symbol.close_group, parse_expr)
  >|= (
    ((expr, _) as expr_node) =>
      Node.typed(
        AST.Raw.of_group(expr),
        Node.get_type(expr),
        Node.get_range(expr_node),
      )
  );
