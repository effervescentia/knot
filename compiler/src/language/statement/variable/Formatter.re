open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (pp_expression, ppf, ((name, _), (expression, _))) =>
    Fmt.pf(ppf, "let %s = %a;", name, pp_expression, expression);
