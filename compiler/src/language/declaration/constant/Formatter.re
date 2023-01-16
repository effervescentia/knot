open Knot.Kore;

let format: Fmt.t((string, KExpression.Interface.node_t('typ))) =
  (ppf, (name, (expression, _))) =>
    Fmt.pf(
      ppf,
      "const %s = %a;",
      name,
      KExpression.Plugin.format,
      expression,
    );
