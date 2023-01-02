open Knot.Kore;

let format: Fmt.t((string, AST.Result.expression_t)) =
  (ppf, (name, (expression, _))) =>
    Fmt.pf(
      ppf,
      "const %s = %a;",
      name,
      KExpression.Plugin.format,
      expression,
    );
