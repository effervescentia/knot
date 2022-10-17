open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let style_rule =
    (
      ctx: ParseContext.t,
      rule_scope: Scope.t,
      parse_expr: contextual_expression_parser_t,
    ) =>
  Identifier.parser(ctx)
  >>= (
    rule =>
      Symbol.colon
      >> parse_expr(ctx)
      >|= (
        expr => {
          N.untyped(
            (N.typed(fst(rule), TR.(`Unknown), N.get_range(rule)), expr),
            N.join_ranges(rule, expr),
          );
        }
      )
  );

let parser =
    (ctx: ParseContext.t, parse_expr: contextual_expression_parser_t)
    : expression_parser_t => {
  let rule_scope = Scope.create(ctx, Range.zero);

  Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);

  Keyword.style
  >>= (
    start =>
      style_rule(ctx, rule_scope, parse_expr)
      |> M.comma_sep
      |> M.between(Symbol.open_closure, Symbol.close_closure)
      >|= (
        raw_rules =>
          N.typed(
            AR.of_style(raw_rules |> fst),
            `Style,
            N.join_ranges(start, raw_rules),
          )
      )
  );
};
