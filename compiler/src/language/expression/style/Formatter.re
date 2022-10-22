open Knot.Kore;

let rec pp_style_rule:
  Fmt.t(AST.raw_expression_t) => Fmt.t(AST.raw_style_rule_t) =
  (pp_expression, ppf, ((key, _), (value, _))) =>
    Fmt.(pf(ppf, "%a,", attribute(string, pp_expression), (key, value)));

let rec pp_style_expression:
  Fmt.t(AST.raw_expression_t) => Fmt.t(list(AST.style_rule_t)) =
  (pp_expression, ppf, rules) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>style %a@]",
        closure(pp_style_rule(pp_expression)),
        rules |> List.map(fst),
      )
    );
