open Knot.Kore;

let format: Fmt.t((string, AST.Result.expression_t)) =
  (ppf, (name, (expr, _))) =>
    Fmt.pf(ppf, "const %s = %a;", name, KExpression.Plugin.format, expr);
