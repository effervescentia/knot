open Kore;
open AST;

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

let _attribute = (~prefix=M.alpha <|> Character.underscore, x) =>
  Operator.assign(
    M.identifier(~prefix) >|= (id => (id |> Block.value, id |> Block.cursor)),
    x,
  )
  >|= (
    ((name, value)) => (
      name,
      Some(value),
      Cursor.join(name |> snd, Tuple.thd3(value)),
    )
  )
  <|> (
    M.identifier(~prefix)
    >|= (
      id => (
        (id |> Block.value, id |> Block.cursor),
        None,
        id |> Block.cursor,
      )
    )
  );

let _self_closing =
  Tag.self_close >|= Block.cursor >|= (end_cursor => ([], end_cursor));

let rec parser = (x, input) =>
  (choice([fragment(x), tag(x)]) |> M.lexeme)(input)
and fragment = x =>
  children(x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= (block => (block |> Block.value |> of_frag, block |> Block.cursor))
and tag = x =>
  Tag.open_
  >> M.identifier
  >>= (
    id =>
      attributes(x)
      >>= (
        attrs =>
          _self_closing
          <|> (
            Tag.close
            >> children(x)
            >>= (
              cs =>
                id
                |> Block.value
                |> M.keyword
                |> M.between(Tag.open_end, Tag.close)
                >|= Block.cursor
                >|= (end_cursor => (cs, end_cursor))
            )
          )
          >|= (
            ((cs, end_cursor)) => (
              (
                (id |> Block.value |> of_public, id |> Block.cursor),
                attrs,
                cs,
              )
              |> of_tag,
              Cursor.join(id |> Block.cursor, end_cursor),
            )
          )
      )
  )
and attributes = x =>
  choice([
    _attribute(x)
    >|= (
      ((name, value, cursor)) => (
        (name |> Tuple.map_fst2(of_public), value) |> of_prop,
        cursor,
      )
    ),
    _attribute(~prefix=Character.period, x)
    >|= (
      ((name, value, cursor)) => (
        (name |> Tuple.map_fst2(String.drop_left(1) % of_public), value)
        |> of_jsx_class,
        cursor,
      )
    ),
    M.identifier(~prefix=Character.octothorp)
    >|= (
      id => (
        (
          id |> Block.value |> String.drop_left(1) |> of_public,
          id |> Block.cursor,
        )
        |> of_jsx_id,
        id |> Block.cursor,
      )
    ),
  ])
  |> many
and children = x =>
  choice([node(x), inline_expr(x), text(x)]) |> M.lexeme |> many
and text = x =>
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
  >|= (
    block => (
      block |> Block.value |> String.trim |> of_text,
      block |> Block.cursor,
    )
  )
and node = x =>
  parser(x) >|= (((_, cursor) as node) => (node |> of_node, cursor))
and inline_expr = x =>
  x
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= (
    block => (block |> Block.value |> of_inline_expr, block |> Block.cursor)
  );
