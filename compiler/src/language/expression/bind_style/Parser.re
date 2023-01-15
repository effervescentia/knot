open Knot.Kore;
open Parse.Kore;

module Glyph = Constants.Glyph;
module ViewKind = KSX.Interface.ViewKind;

let parse_bind_style_expression = f =>
  Parse.Util.binary_op(((lhs, rhs)) => (ViewKind.Element, lhs, rhs) |> f)
  <$ Matchers.glyph(Glyph.style_binding);

let parse_bind_style_literal =
    (
      f: Interface.Plugin.value_t('expr, 'typ) => 'expr,
      (ctx, (_, parse_style)): Interface.Plugin.parse_arg_t('ast, 'expr),
      view: Node.t('expr, unit),
    ) =>
  Matchers.glyph(Glyph.style_binding)
  >> parse_style(ctx)
  >|= (
    style =>
      Node.raw(
        (ViewKind.Element, view, style) |> f,
        Node.join_ranges(view, style),
      )
  );

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, (ctx, (parse_view, _)) as arg) =>
    parse_view(ctx)
    >>= parse_bind_style_literal(f, arg)
    <|> chainl1(parse_view(ctx), parse_bind_style_expression(f));
