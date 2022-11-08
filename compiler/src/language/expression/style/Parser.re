open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module ParseContext = AST.ParseContext;
module Scope = AST.Scope;
module Matchers = Parse.Matchers;

let style_rule =
    (ctx: ParseContext.t, parse_expr: contextual_expression_parser_t) =>
  Matchers.attribute(KIdentifier.Plugin.parse(ctx), parse_expr(ctx))
  >|= (
    ((rule, expr)) => {
      Node.untyped(
        (Node.typed(fst(rule), (), Node.get_range(rule)), expr),
        Node.join_ranges(rule, expr),
      );
    }
  );

let style_expression =
    (ctx: ParseContext.t, parse_expr: contextual_expression_parser_t)
    : expression_parser_t => {
  let rule_scope = Scope.create(ctx, Range.zero);

  Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);

  Matchers.keyword(Constants.Keyword.style)
  >>= (
    start =>
      style_rule(ctx, parse_expr)
      |> Matchers.comma_sep
      |> Matchers.between_braces
      >|= (
        raw_rules =>
          Node.typed(
            AST.Raw.of_style(raw_rules |> fst),
            (),
            Node.join_ranges(start, raw_rules),
          )
      )
  );
};
