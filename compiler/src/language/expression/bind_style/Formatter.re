open Knot.Kore;
open AST;

let pp_bind_style:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.expression_t, Result.expression_t)) =
  (pp_expression, ppf, ((view, _), (style, _))) =>
    Fmt.pf(
      ppf,
      "%a::%a",
      pp_expression,
      view,
      ppf =>
        Expression.(
          fun
          | Style(rules) =>
            KStyle.Plugin.pp_style_rules(pp_expression, ppf, rules)
          | expr => pp_expression(ppf, expr)
        ),
      style,
    );
