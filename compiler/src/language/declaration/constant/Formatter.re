open Knot.Kore;

let format: Interface.Plugin.format_t('typ) =
  (ppf, (name, (expression, _))) =>
    Fmt.pf(
      ppf,
      "const %s = %a;",
      name,
      KExpression.Plugin.format,
      expression,
    );
