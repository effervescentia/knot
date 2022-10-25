open Knot.Kore;

let pp_constant: Fmt.t((string, AST.expression_t)) =
  (ppf, (name, (expr, _))) =>
    Fmt.pf(ppf, "const %s = %a;", name, KExpression.Plugin.pp, expr);
