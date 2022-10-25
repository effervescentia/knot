open Knot.Kore;

let pp_dot_access:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.expression_t, AST.untyped_t(string))) =
  (pp_expression, ppf, ((expr, _), (prop, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, expr, prop);
