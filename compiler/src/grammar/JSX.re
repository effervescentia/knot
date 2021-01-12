open Kore;

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
    M.identifier(~prefix) >|= Block.value,
    x >|= (v => Some(v)),
  )
  <|> (M.identifier(~prefix) >|= Block.value >|= (s => (s, None)));

let _self_closing = [] <$ Tag.self_close;

let rec parser = (x, input) =>
  (choice([fragment(x), tag(x)]) |> M.lexeme)(input)
and fragment = x =>
  children(x)
  |> M.between(Fragment.open_, Fragment.close)
  >|= Block.value
  >|= AST.of_frag
and tag = x =>
  Character.open_chevron
  >> M.identifier
  >|= Block.value
  >>= (
    id =>
      attributes(x)
      >>= (
        attrs =>
          _self_closing
          <|> (
            M.lexeme(Character.close_chevron)
            >> children(x)
            << (M.keyword(id) |> M.between(Tag.open_end, Tag.close))
          )
          >|= (cs => AST.of_tag((id, attrs, cs)))
      )
  )
and attributes = x =>
  choice([
    _attribute(x) >|= AST.of_prop,
    _attribute(~prefix=Character.period, x)
    >|= Tuple.map_fst2(s => String.sub(s, 1, String.length(s) - 1))
    >|= AST.of_jsx_class,
    _attribute(~prefix=Character.octothorp, x)
    >|= Tuple.map_fst2(s => String.sub(s, 1, String.length(s) - 1))
    >|= AST.of_jsx_id,
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
  >|= AST.of_text
and node = x => parser(x) >|= AST.of_node
and inline_expr = x =>
  x
  |> M.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Block.value
  >|= AST.of_inline_expr;
