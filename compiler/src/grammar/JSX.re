open Kore;

let _attribute = (~prefix=M.alpha <|> Character.underscore, x) =>
  M.binary_op(
    M.identifier(~prefix) >|= Block.value,
    Operator.assign,
    x >|= (v => Some(v)),
  )
  <|> (M.identifier(~prefix) >|= Block.value % (s => (s, None)));

let _self_closing = Glyph.self_close_tag >> return([]);

let rec parser = (x, input) =>
  (choice([fragment(x), tag(x)]) |> M.lexeme)(input)
and fragment = x =>
  children(x)
  |> M.between(Glyph.open_fragment, Glyph.close_fragment)
  >|= AST.of_frag
and tag = x =>
  Character.open_chevron
  >> M.identifier
  >>= Block.value
  % (
    id =>
      attributes(x)
      >>= (
        attrs =>
          _self_closing
          <|> (
            M.lexeme(Character.close_chevron)
            >> children(x)
            << (
              M.token(id)
              |> M.between(Glyph.close_tag, Character.close_chevron)
            )
          )
          >|= (cs => AST.of_tag((id, attrs, cs)))
      )
  )
and attributes = x =>
  choice([
    _attribute(x) >|= AST.of_prop,
    _attribute(~prefix=Character.period, x)
    >|= Tuple.map_fst2(s => String.sub(s, 1, String.length(s) - 1))
    % AST.of_jsx_class,
    _attribute(~prefix=Character.octothorp, x)
    >|= Tuple.map_fst2(s => String.sub(s, 1, String.length(s) - 1))
    % AST.of_jsx_id,
  ])
  |> many
and children = x =>
  choice([node(x), inline_expr(x), text(x)]) |> M.lexeme |> many
and text = x =>
  none_of([Constants.Character.open_brace, Constants.Character.open_chevron])
  <~> (
    none_of([
      Constants.Character.open_brace,
      Constants.Character.open_chevron,
      Constants.Character.close_chevron,
    ])
    |> many
  )
  >|= Char.join
  % Block.value
  % String.trim
  % AST.of_text
and node = x => parser(x) >|= AST.of_node
and inline_expr = x =>
  x
  |> M.between(Character.open_brace, Character.close_brace)
  >|= AST.of_inline_expr;
