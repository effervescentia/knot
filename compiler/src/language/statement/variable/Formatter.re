open Knot.Kore;

let pp_variable:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.untyped_t(string), AST.expression_t)) =
  (pp_expression, ppf, ((name, _), (expr, _))) =>
    Fmt.pf(ppf, "let %s = %a;", name, pp_expression, expr);
