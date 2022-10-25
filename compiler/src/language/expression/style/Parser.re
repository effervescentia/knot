open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let style_rule =
    (
      ctx: ParseContext.t,
      rule_scope: Scope.t,
      parse_expr: Grammar.Kore.contextual_expression_parser_t,
    ) =>
  KIdentifier.Plugin.parse(ctx)
  >>= (
    rule =>
      Symbol.colon
      >> parse_expr(ctx)
      >|= (
        expr => {
          Node.untyped(
            (
              Node.typed(
                fst(rule),
                Type.Raw.(`Unknown),
                Node.get_range(rule),
              ),
              expr,
            ),
            Node.join_ranges(rule, expr),
          );
        }
      )
  );

let style_expression =
    (
      ctx: ParseContext.t,
      parse_expr: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.expression_parser_t => {
  let rule_scope = Scope.create(ctx, Range.zero);

  Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);

  Keyword.style
  >>= (
    start =>
      style_rule(ctx, rule_scope, parse_expr)
      |> Matchers.comma_sep
      |> Matchers.between(Symbol.open_closure, Symbol.close_closure)
      >|= (
        raw_rules =>
          Node.typed(
            AST.Raw.of_style(raw_rules |> fst),
            `Style,
            Node.join_ranges(start, raw_rules),
          )
      )
  );
};
