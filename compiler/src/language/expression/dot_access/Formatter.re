open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, pp_expression, ppf, ((object_, _), (property, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, object_, property);
