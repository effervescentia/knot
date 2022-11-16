open Knot.Kore;
open Parse.Kore;
open AST;

let style_rule =
    (
      ctx: ParseContext.t,
      parse_expr: Framework.contextual_expression_parser_t,
    ) =>
  Matchers.attribute(KIdentifier.Plugin.parse_id(ctx), parse_expr(ctx))
  >|= (
    ((rule, expr)) => {
      Node.untyped(
        (Node.typed(fst(rule), (), Node.get_range(rule)), expr),
        Node.join_ranges(rule, expr),
      );
    }
  );

let style_literal =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    )
    : Framework.expression_parser_t => {
  let rule_scope = Scope.create(ctx, Range.zero);

  Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);

  style_rule(ctx, parse_expr)
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(Raw.of_style);
};

let style_expression =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    )
    : Framework.expression_parser_t =>
  Matchers.keyword(Constants.Keyword.style)
  >>= (
    start =>
      style_literal((ctx, parse_expr))
      >|= Node.map_range(Range.join(Node.get_range(start)))
  );
