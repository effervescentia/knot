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
    >|= (((expr, range)) => N.map_range(_ => range, expr))
    <|> parse_term(ctx),
  )
  >|= (
    ((name, value)) => (
      name,
      Some(value),
      Range.join(NR.get_range(name), N.get_range(value)),
    )
  )
  <|> (M.identifier(~prefix) >|= (id => (id, None, NR.get_range(id))));

let _self_closing = Tag.self_close >|= NR.map_value(() => []);

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
  >|= (
    ((xs, range)) => N.create(AR.of_frag(xs), TR.Valid(`Element), range)
  )

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
              |> NR.get_value
              |> M.keyword
              |> M.between(Tag.open_end, Tag.close)
              >|= NR.get_range
              >|= NR.create(cs)
          )
          <|> _self_closing
          >|= (
            cs =>
              N.create(
                (
                  id |> NR.map_value(Identifier.of_string),
                  attrs,
                  NR.get_value(cs),
                )
                |> AR.of_tag,
                TR.Valid(`Element),
                NR.join_ranges(id, cs),
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
      N.create(
        (name |> NR.map_value(AR.of_public), value) |> AR.of_prop,
        value |> Option.map(N.get_type) |?: TR.Valid(`Unknown),
        range,
      )
  )

and class_attribute =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _attribute(~prefix=Character.period, ctx, parsers)
  >|= (
    ((name, value, range)) =>
      N.create(
        (name |> NR.map_value(String.drop_left(1) % AR.of_public), value)
        |> AR.of_jsx_class,
        /* classes are dynamic boolean values */
        TR.Valid(`Boolean),
        range,
      )
  )

and id_attribute: jsx_attribute_parser_t =
  M.identifier(~prefix=Character.octothorpe)
  >|= NR.map_value(String.drop_left(1) % AR.of_public)
  >|= NR.wrap(AR.of_jsx_id)
  /* identifiers are static string values */
  >|= N.of_raw(TR.Valid(`String))

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
  >|= NR.map_value(String.trim)
  >|= N.of_raw(TR.Valid(`String))
  >|= N.map_value(AR.of_text)

and node =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parser(ctx, parsers) >|= N.wrap(AR.of_node)

and inline_expr =
    (ctx: ModuleContext.t, (_, parse_expr): expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parse_expr(ctx)
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= (
    ((expr, range)) =>
      N.create(AR.of_inline_expr(expr), N.get_type(expr), range)
  );
