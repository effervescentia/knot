open Core;

let root =
  (
    fun
    | JSXStartTag => JSX.StartTag.lexer
    | JSXEndTag => JSX.EndTag.lexer
    | JSXContent => JSX.Content.lexer
    | Normal =>
      Lexers([
        Character.lexer,
        Keyword.lexer,
        Pattern.lexer,
        Text.lexer,
        Identifier.lexer(),
        Number.lexer,
        Boolean.lexer,
        Comment.lexer,
      ])
  )
  % Util.normalize_lexers;

let rec exec_lexer = (results, s, lex, stream) =>
  switch (lex) {
  | Lexers(ls) =>
    Lexers(
      List.fold_left(
        (acc, l) =>
          switch (exec_lexer(results, s, l, stream)) {
          | Some(r) => [r, ...acc]
          | None => acc
          },
        [],
        ls,
      ),
    )
    |> Util.normalize_lexers
  | Lexer(m, nm, t) =>
    Util.test_match(m, stream, next_stream =>
      Util.test_match(nm, next_stream, _ => t(s) |> Util.normalize_lexers)
    )
  | Result(tkn) =>
    results := [(tkn, stream), ...results^];
    None;
  }
and find_token = (~results=ref([]), ~token_str="", lex, stream) =>
  switch (lex, stream) {
  /* found a single longest result */
  | (Some(Result(tkn)), _) => Some((tkn, stream))
  /* found a lexer */
  | (Some(l), LazyStream.Cons((ch, _), next_stream)) =>
    let next_string = token_str ++ String.make(1, ch);

    switch (exec_lexer(results, next_string, l, stream)) {
    /* remaining lexers */
    | Some(_) as res =>
      find_token(
        ~results,
        ~token_str=next_string,
        res,
        Lazy.force(next_stream),
      )
    /* no remaining lexers */
    | None => find_token(~results, ~token_str, None, stream)
    };
  /* ran out of lexers */
  | (None, _) =>
    switch (results^) {
    | [x, ...xs] => Some(x)
    | [] => None
    }
  /* hit EOF */
  | (Some(l), LazyStream.Nil) =>
    switch (Util.find_result(None, l)) {
    | Some(r) => Some((r, stream))
    | None => None
    }
  };

let next_token = ctx => find_token(root(ctx));
