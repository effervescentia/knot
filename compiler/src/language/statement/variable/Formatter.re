open Knot.Kore;
open AST;

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Common.identifier_t, Result.expression_t)) =
  (pp_expression, ppf, ((name, _), (expr, _))) =>
    Fmt.pf(ppf, "let %s = %a;", name, pp_expression, expr);
