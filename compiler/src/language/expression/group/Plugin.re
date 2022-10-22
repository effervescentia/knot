open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let parse =
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

let pp: Fmt.t(AST.raw_expression_t) => Fmt.t(AST.expression_t) =
  (pp_expression, ppf, (expr, _)) =>
    switch (expr) {
    | BinaryOp(_) => Fmt.pf(ppf, "(%a)", pp_expression, expr)

    /* collapse parentheses around all other values */
    | _ => pp_expression(ppf, expr)
    };
