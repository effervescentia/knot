open Knot.Kore;
open Parse.Kore;
open AST;

module Glyph = Constants.Glyph;

let parse_bind_style_expression: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_component_bind_style)
  <$ Matchers.glyph(Glyph.style_binding);

let parse_bind_style_literal = ((ctx, parse_expr), view) =>
  Matchers.glyph(Glyph.style_binding)
  >> KStyle.Parser.parse_style_literal((ctx, parse_expr))
  >|= (
    style =>
      Node.raw(
        (view, style) |> Raw.of_component_bind_style,
        Node.join_ranges(view, style),
      )
  );

let parse =
    (
      (
        ctx: ParseContext.t,
        (parse_lhs, parse_expr): (
          Framework.contextual_expression_parser_t,
          Framework.contextual_expression_parser_t,
        ),
      ),
    ) =>
  parse_lhs(ctx)
  >>= parse_bind_style_literal((ctx, parse_expr))
  <|> chainl1(parse_lhs(ctx), parse_bind_style_expression);
