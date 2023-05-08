open Knot.Kore;

let format_style_rule:
  Fmt.t('expr) => Fmt.t(Interface.StyleRule.t('expr, 'typ)) =
  (format_expression, ppf, ((key, _), (value, _))) =>
    Fmt.(
      pf(ppf, "%a,", attribute(string, format_expression), (key, value))
    );

let format_style_rule_list:
  Fmt.t('expr) => Fmt.t(list(Interface.StyleRule.node_t('expr, 'typ))) =
  (format_expression, ppf, rules) =>
    Fmt.(
      closure(
        format_style_rule(format_expression),
        ppf,
        rules |> List.map(fst),
      )
    );

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, format_expression, ppf, rules) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>style %a@]",
        format_style_rule_list(format_expression),
        rules,
      )
    );
