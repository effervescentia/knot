open Kore;
open AST.Raw;

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
      (term, expr),
    ) =>
  Operator.assign(
    M.identifier(~prefix) >|= Node.Raw.(Tuple.split2(get_value, get_range)),
    expr(ctx)
    |> M.between(Symbol.open_group, Symbol.close_group)
    >|= Node.Raw.get_value
    <|> term(ctx),
  )
  >|= (
    ((name, value)) => (
      name,
      Some(value),
      Range.join(snd(name), Node.Raw.get_range(value)),
    )
  )
  <|> (
    M.identifier(~prefix)
    >|= (
      id => (
        id |> Node.Raw.(Tuple.split2(get_value, get_range)),
        None,
        Node.Raw.get_range(id),
      )
    )
  );

let _self_closing =
  Tag.self_close >|= Node.Raw.get_range >|= (end_range => ([], end_range));

let rec parser = (ctx: ModuleContext.t, x, input) =>
  (choice([fragment(ctx, x), tag(ctx, x)]) |> M.lexeme)(input)

and fragment = (ctx: ModuleContext.t, x) =>
  children(ctx, x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= Node.Raw.(Tuple.split2(get_value % of_frag, get_range))

and tag = (ctx: ModuleContext.t, x) =>
  Tag.open_
  >> M.identifier
  >>= (
    id =>
      attributes(ctx, x)
      >>= (
        attrs =>
          Tag.close
          >> children(ctx, x)
          >>= (
            cs =>
              id
              |> Node.Raw.get_value
              |> M.keyword
              |> M.between(Tag.open_end, Tag.close)
              >|= Node.Raw.get_range
              >|= (end_range => (cs, end_range))
          )
          <|> _self_closing
          >|= (
            ((cs, end_range)) => (
              (
                id
                |> Node.Raw.(
                     Tuple.split2(
                       get_value % Reference.Identifier.of_string,
                       get_range,
                     )
                   ),
                attrs,
                cs,
              )
              |> of_tag,
              Range.join(Node.Raw.get_range(id), end_range),
            )
          )
      )
  )

and attributes = (ctx: ModuleContext.t, x) =>
  choice([
    _attribute(ctx, x)
    >|= (
      ((name, value, range)) => (
        (name |> Tuple.map_fst2(of_public), value) |> of_prop,
        range,
      )
    ),
    _attribute(~prefix=Character.period, ctx, x)
    >|= (
      ((name, value, range)) => (
        (name |> Tuple.map_fst2(String.drop_left(1) % of_public), value)
        |> of_jsx_class,
        range,
      )
    ),
    M.identifier(~prefix=Character.octothorpe)
    >|= Node.Raw.(
          Tuple.split2(
            Tuple.split2(
              get_value % String.drop_left(1) % of_public,
              get_range,
            )
            % of_jsx_id,
            get_range,
          )
        ),
  ])
  |> many

and children = (ctx: ModuleContext.t, x) =>
  choice([node(ctx, x), inline_expr(ctx, x), text]) |> M.lexeme |> many

and text =
  none_of(C.Character.[open_brace, open_chevron])
  <~> (
    none_of(C.Character.[open_brace, open_chevron, close_chevron]) |> many
  )
  >|= Input.join
  >|= Node.Raw.(
        Tuple.split2(
          Tuple.split2(get_value % String.trim, get_range) % of_text,
          get_range,
        )
      )

and node = (ctx: ModuleContext.t, x) =>
  parser(ctx, x) >|= (node => (of_node(node), Node.Raw.get_range(node)))

and inline_expr = (ctx: ModuleContext.t, (_, expr)) =>
  expr(ctx)
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Node.Raw.(Tuple.split2(get_value % of_inline_expr, get_range));
