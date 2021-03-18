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
    x >|= (v => Some(v)),
  )
  <|> (
    M.identifier(~prefix)
    >|= (id => ((id |> Block.value, id |> Block.cursor), None))
  );

let _self_closing = [] <$ Tag.self_close;

let rec parser = (x, input) =>
  (choice([fragment(x), tag(x)]) |> M.lexeme)(input)
and fragment = x =>
  children(x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= Block.value
  >|= of_frag
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
            << (
              id
              |> Block.value
              |> M.keyword
              |> M.between(Tag.open_end, Tag.close)
            )
          )
          >|= (
            cs =>
              of_tag((
                (id |> Block.value |> of_public, id |> Block.cursor),
                attrs,
                cs,
              ))
          )
      )
  )
and attributes = x =>
  choice([
    _attribute(x) >|= Tuple.map_fst2(Tuple.map_fst2(of_public)) >|= of_prop,
    _attribute(~prefix=Character.period, x)
    >|= Tuple.map_fst2(Tuple.map_fst2(String.drop_left(1) % of_public))
    >|= of_jsx_class,
    M.identifier(~prefix=Character.octothorp)
    >|= (
      id => (
        id |> Block.value |> String.drop_left(1) |> of_public,
        id |> Block.cursor,
      )
    )
    >|= of_jsx_id,
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
  >|= Char.join
  >|= Block.value
  >|= String.trim
  >|= of_text
and node = x => parser(x) >|= of_node
and inline_expr = x =>
  x
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Block.value
  >|= of_inline_expr;
