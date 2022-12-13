open Knot.Kore;
open AST;

let format_style_rule:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_style_rule_t) =
  (format_expression, ppf, ((key, _), (value, _))) =>
    Fmt.(
      pf(ppf, "%a,", attribute(string, format_expression), (key, value))
    );

let format_style_rules:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.style_rule_t)) =
  (format_expression, ppf, rules) =>
    Fmt.(
      closure(
        format_style_rule(format_expression),
        ppf,
        rules |> List.map(fst),
      )
    );

let format:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.style_rule_t)) =
  (format_expression, ppf, rules) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>style %a@]",
        format_style_rules(format_expression),
        rules,
      )
    );
