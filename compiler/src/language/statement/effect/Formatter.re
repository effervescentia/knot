open Knot.Kore;

let pp_effect:
  Fmt.t(AST.Result.raw_expression_t) => Fmt.t(AST.Result.expression_t) =
  (pp_expression, ppf, (expr, _)) =>
    Fmt.pf(ppf, "%a;", pp_expression, expr);
