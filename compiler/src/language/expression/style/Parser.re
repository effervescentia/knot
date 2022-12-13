open Knot.Kore;
open Parse.Kore;
open AST;

let parse_style_rule =
    (
      ctx: ParseContext.t,
      parse_expr: Framework.contextual_expression_parser_t,
    ) =>
  Matchers.attribute(KIdentifier.Parser.parse_raw(ctx), parse_expr(ctx))
  >|= (
    ((rule, expr)) => {
      Node.untyped(
        (Node.typed(fst(rule), (), Node.get_range(rule)), expr),
        Node.join_ranges(rule, expr),
      );
    }
  );

let parse_style_literal =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    )
    : Framework.expression_parser_t =>
  parse_style_rule(ctx, parse_expr)
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(Raw.of_style);

let parse =
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
      parse_style_literal((ctx, parse_expr))
      >|= Node.map_range(Range.join(Node.get_range(start)))
  );
