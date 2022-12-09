open Knot.Kore;
open Parse.Kore;
open AST;

module Glyph = Constants.Glyph;

let parse_bind_style_expression: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_local_bind_style)
  <$ Matchers.glyph(Glyph.style_binding);

let parse_bind_style_literal = ((ctx, parse_expr)) =>
  parse_expr(ctx)
  >>= (
    expr =>
      Matchers.glyph(Glyph.style_binding)
      >> KStyle.Parser.parse_style_literal((ctx, parse_expr))
      >|= (
        literal =>
          Node.untyped(
            (expr, literal) |> Raw.of_local_bind_style,
            Node.join_ranges(expr, literal),
          )
      )
  );

let parse =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    ) =>
  parse_bind_style_literal((ctx, parse_expr))
  <|> chainl1(parse_expr(ctx), parse_bind_style_expression);
