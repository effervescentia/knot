open Knot.Kore;
open Parse.Kore;

let parse_style_rule =
    ((ctx, parse_expr): Interface.Plugin.parse_arg_t('ast, 'expr)) =>
  Matchers.attribute(KIdentifier.Plugin.parse_raw(ctx), parse_expr(ctx))
  >|= (
    ((rule, expr)) => {
      Node.raw((rule, expr), Node.join_ranges(rule, expr));
    }
  );

let parse_style_literal =
    (f, (ctx, parse_expr): Interface.Plugin.parse_arg_t('ast, 'expr)) =>
  parse_style_rule((ctx, parse_expr))
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(f);

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, (ctx, parse_expr)) =>
    Matchers.keyword(Constants.Keyword.style)
    >>= (
      start =>
        parse_style_literal(f, (ctx, parse_expr))
        >|= Node.map_range(Range.join(Node.get_range(start)))
    );
