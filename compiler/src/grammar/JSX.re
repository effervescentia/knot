open Kore;

module Identifier = Reference.Identifier;

module Tag = {
  let open_ = M.symbol(C.Character.open_chevron);
  let close = M.symbol(C.Character.close_chevron);

  let self_close = M.glyph(C.Glyph.self_close_tag);
  let open_end = M.glyph(C.Glyph.open_end_tag);
};

module Fragment = {
  let open_ = M.glyph(C.Glyph.open_fragment);
  let close = M.glyph(C.Glyph.close_fragment);
};

let _attribute =
    (
      ~prefix=M.alpha <|> Character.underscore,
      ctx: ModuleContext.t,
      (parse_term, parse_expr): expression_parsers_arg_t,
    ) =>
  Operator.assign(
    M.identifier(~prefix),
    parse_expr(ctx)
    |> M.between(Symbol.open_group, Symbol.close_group)
    >|= (
      ((expr, _) as expr_group) =>
        N.map_range(_ => N.get_range(expr_group), expr)
    )
    <|> parse_term(ctx),
  )
  >|= (
    ((name, value)) => (name, Some(value), N.join_ranges(name, value))
  )
  <|> (M.identifier(~prefix) >|= (id => (id, None, N.get_range(id))));

let _self_closing = Tag.self_close >|= N.map(() => []);

let rec parser =
        (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
        : jsx_parser_t =>
  /* do not attempt to simplify this `input` argument away or JSX parsing will loop forever */
  input =>
    (choice([fragment(ctx, parsers), tag(ctx, parsers)]) |> M.lexeme)(
      input,
    )

and fragment =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  children(ctx, parsers)
  |> M.between(Fragment.open_, Fragment.close)
  >|= N.map(AR.of_frag)

and tag =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  Tag.open_
  >> M.identifier
  >>= (
    id =>
      attributes(ctx, parsers)
      >>= (
        attrs =>
          Tag.close
          >> children(ctx, parsers)
          >>= (
            cs =>
              id
              |> fst
              |> M.keyword
              |> M.between(Tag.open_end, Tag.close)
              >|= N.map(_ => cs)
          )
          <|> _self_closing
          >|= (
            cs =>
              N.untyped(
                (id |> N.map(Identifier.of_string), attrs, fst(cs))
                |> AR.of_tag,
                N.join_ranges(id, cs),
              )
          )
      )
  )

and property_attribute =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _attribute(ctx, parsers)
  >|= (
    ((name, value, range)) =>
      N.untyped((name |> N.map(AR.of_public), value) |> AR.of_prop, range)
  )

and class_attribute =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _attribute(~prefix=Character.period, ctx, parsers)
  >|= (
    ((name, value, range)) =>
      N.untyped(
        (name |> N.map(String.drop_left(1) % AR.of_public), value)
        |> AR.of_jsx_class,
        range,
      )
  )

and id_attribute: jsx_attribute_parser_t =
  M.identifier(~prefix=Character.octothorpe)
  >|= N.map(String.drop_left(1) % AR.of_public)
  >|= N.wrap(AR.of_jsx_id)

and attributes =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_list_parser_t =>
  choice([
    property_attribute(ctx, parsers),
    class_attribute(ctx, parsers),
    id_attribute,
  ])
  |> many

and children =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_list_parser_t =>
  choice([node(ctx, parsers), inline_expr(ctx, parsers), text])
  |> M.lexeme
  |> many

and text: jsx_child_parser_t =
  none_of(C.Character.[open_brace, open_chevron])
  <~> (
    none_of(C.Character.[open_brace, open_chevron, close_chevron]) |> many
  )
  >|= Input.join
  >|= N.map(String.trim % AR.of_text)

and node =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parser(ctx, parsers) >|= N.map(AR.of_node)

and inline_expr =
    (ctx: ModuleContext.t, (_, parse_expr): expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parse_expr(ctx)
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= N.map(AR.of_inline_expr);
