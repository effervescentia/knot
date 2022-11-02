open Knot.Kore;

let pp_variable:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t((AST.Result.untyped_t(string), AST.Result.expression_t)) =
  (pp_expression, ppf, ((name, _), (expr, _))) =>
    Fmt.pf(ppf, "let %s = %a;", name, pp_expression, expr);
