open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (get_style_rules, pp_expression, ppf, (_, (view, _), (style, _))) =>
    Fmt.pf(
      ppf,
      "%a::%a",
      pp_expression,
      view,
      (ppf, expr) =>
        switch (get_style_rules(expr)) {
        | Some(rules) =>
          KStyle.Formatter.format_style_rule_list(pp_expression, ppf, rules)
        | None => pp_expression(ppf, expr)
        },
      style,
    );
