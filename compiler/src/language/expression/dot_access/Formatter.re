open Knot.Kore;
open AST;

let pp_dot_access:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.expression_t, Result.untyped_t(string))) =
  (pp_expression, ppf, ((expr, _), (prop, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, expr, prop);
