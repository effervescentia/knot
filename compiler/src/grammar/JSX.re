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
      ctx: ClosureContext.t,
      (term, expr),
    ) =>
  Operator.assign(
    M.identifier(~prefix) >|= Tuple.split2(Block.value, Block.cursor),
    expr(ctx)
    |> M.between(Symbol.open_group, Symbol.close_group)
    >|= Block.value
    <|> term(ctx),
  )
  >|= (
    ((name, value)) => (
      name,
      Some(value),
      Cursor.join(snd(name), Block.cursor(value)),
    )
  )
  <|> (
    M.identifier(~prefix)
    >|= (
      id => (
        id |> Tuple.split2(Block.value, Block.cursor),
        None,
        Block.cursor(id),
      )
    )
  );

let _self_closing =
  Tag.self_close >|= Block.cursor >|= (end_cursor => ([], end_cursor));

let rec parser = (ctx: ClosureContext.t, x, input) =>
  (choice([fragment(ctx, x), tag(ctx, x)]) |> M.lexeme)(input)

and fragment = (ctx: ClosureContext.t, x) =>
  children(ctx, x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= Tuple.split2(Block.value % of_frag, Block.cursor)

and tag = (ctx: ClosureContext.t, x) =>
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
              |> Block.value
              |> M.keyword
              |> M.between(Tag.open_end, Tag.close)
              >|= Block.cursor
              >|= (end_cursor => (cs, end_cursor))
          )
          <|> _self_closing
          >|= (
            ((cs, end_cursor)) => (
              (
                id
                |> Tuple.split2(
                     Block.value % Reference.Identifier.of_string,
                     Block.cursor,
                   ),
                attrs,
                cs,
              )
              |> of_tag,
              Cursor.join(Block.cursor(id), end_cursor),
            )
          )
      )
  )

and attributes = (ctx: ClosureContext.t, x) =>
  choice([
    _attribute(ctx, x)
    >|= (
      ((name, value, cursor)) => (
        (name |> Tuple.map_fst2(of_public), value) |> of_prop,
        cursor,
      )
    ),
    _attribute(~prefix=Character.period, ctx, x)
    >|= (
      ((name, value, cursor)) => (
        (name |> Tuple.map_fst2(String.drop_left(1) % of_public), value)
        |> of_jsx_class,
        cursor,
      )
    ),
    M.identifier(~prefix=Character.octothorp)
    >|= Tuple.split2(
          Tuple.split2(
            Block.value % String.drop_left(1) % of_public,
            Block.cursor,
          )
          % of_jsx_id,
          Block.cursor,
        ),
  ])
  |> many

and children = (ctx: ClosureContext.t, x) =>
  choice([node(ctx, x), inline_expr(ctx, x), text]) |> M.lexeme |> many

and text =
  none_of([C.Character.open_brace, C.Character.open_chevron])
  <~> (
    none_of([
      C.Character.open_brace,
      C.Character.open_chevron,
      C.Character.close_chevron,
    ])
    |> many
  )
  >|= Input.join
  >|= Tuple.split2(
        Tuple.split2(Block.value % String.trim, Block.cursor) % of_text,
        Block.cursor,
      )

and node = (ctx: ClosureContext.t, x) =>
  parser(ctx, x) >|= (((_, cursor) as node) => (of_node(node), cursor))

and inline_expr = (ctx: ClosureContext.t, (_, expr)) =>
  expr(ctx)
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Tuple.split2(Block.value % of_inline_expr, Block.cursor);
