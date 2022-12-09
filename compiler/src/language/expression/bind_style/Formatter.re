open Knot.Kore;
open AST;

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.bind_style_target_t, Result.expression_t)) =
  (pp_expression, ppf, (view, (style, _))) =>
    Fmt.pf(
      ppf,
      "%a::%a",
      pp_expression,
      switch (view) {
      | BuiltIn((view', _))
      | Local((view', _)) => view'
      },
      ppf =>
        Expression.(
          fun
          | Style(rules) =>
            KStyle.Formatter.format_style_rules(pp_expression, ppf, rules)
          | expr => pp_expression(ppf, expr)
        ),
      style,
    );
