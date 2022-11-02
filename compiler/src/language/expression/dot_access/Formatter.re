open Knot.Kore;

let pp_dot_access:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t((AST.Result.expression_t, AST.Result.untyped_t(string))) =
  (pp_expression, ppf, ((expr, _), (prop, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, expr, prop);
