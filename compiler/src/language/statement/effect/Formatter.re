open Knot.Kore;

let pp_effect: Fmt.t(AST.raw_expression_t) => Fmt.t(AST.expression_t) =
  (pp_expression, ppf, (expr, _)) =>
    Fmt.pf(ppf, "%a;", pp_expression, expr);
