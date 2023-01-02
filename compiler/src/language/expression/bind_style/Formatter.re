open Knot.Kore;
open AST;

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Expression.view_source_t, Result.expression_t, Result.expression_t)) =
  (pp_expression, ppf, (_, (view, _), (style, _))) =>
    Fmt.pf(
      ppf,
      "%a::%a",
      pp_expression,
      view,
      ppf =>
        Expression.(
          fun
          | Style(rules) =>
            KStyle.Formatter.format_style_rules(pp_expression, ppf, rules)
          | expr => pp_expression(ppf, expr)
        ),
      style,
    );
