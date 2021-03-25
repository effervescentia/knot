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

let _attribute = (~prefix=M.alpha <|> Character.underscore, scope, x) =>
  Operator.assign(
    M.identifier(~prefix) >|= Tuple.split2(Block.value, Block.cursor),
    x(scope),
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
        id |> Tuple.split2(Block.value, Block.cursor),
        None,
        id |> Block.cursor,
      )
    )
  );

let _self_closing =
  Tag.self_close >|= Block.cursor >|= (end_cursor => ([], end_cursor));

let rec parser = (scope: Scope.t, x, input) =>
  (choice([fragment(scope, x), tag(scope, x)]) |> M.lexeme)(input)
and fragment = (scope: Scope.t, x) =>
  children(scope, x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= Tuple.split2(Block.value % of_frag, Block.cursor)
and tag = (scope: Scope.t, x) =>
  Tag.open_
  >> M.identifier
  >>= (
    id =>
      attributes(scope, x)
      >>= (
        attrs =>
          _self_closing
          <|> (
            Tag.close
            >> children(scope, x)
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
                id
                |> Tuple.split2(
                     Block.value % Reference.Identifier.of_string,
                     Block.cursor,
                   ),
                attrs,
                cs,
              )
              |> of_tag,
              Cursor.join(id |> Block.cursor, end_cursor),
            )
          )
      )
  )
and attributes = (scope: Scope.t, x) =>
  choice([
    _attribute(scope, x)
    >|= (
      ((name, value, cursor)) => (
        (name |> Tuple.map_fst2(of_public), value) |> of_prop,
        cursor,
      )
    ),
    _attribute(~prefix=Character.period, scope, x)
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
and children = (scope: Scope.t, x) =>
  choice([node(scope, x), inline_expr(scope, x), text(scope, x)])
  |> M.lexeme
  |> many
and text = (scope: Scope.t, x) =>
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
  >|= Tuple.split2(Block.value % String.trim % of_text, Block.cursor)
and node = (scope: Scope.t, x) =>
  parser(scope, x) >|= (((_, cursor) as node) => (node |> of_node, cursor))
and inline_expr = (scope: Scope.t, x) =>
  x(scope)
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Tuple.split2(Block.value % of_inline_expr, Block.cursor);
