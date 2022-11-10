open Knot.Kore;
open AST;

let pp_effect: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.expression_t) =
  (pp_expression, ppf, (expr, _)) =>
    Fmt.pf(ppf, "%a;", pp_expression, expr);
