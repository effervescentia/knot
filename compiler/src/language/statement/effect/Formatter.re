open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (pp_expression, ppf, (expression, _)) =>
    Fmt.pf(ppf, "%a;", pp_expression, expression);
