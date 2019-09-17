open Core;

let token_buffer_size = 512;

let root =
  Lexers([
    Character.lexer,
    Keyword.lexer,
    Pattern.lexer,
    Text.lexer,
    Identifier.lexer,
    Number.lexer,
    Boolean.lexer,
    Comment.lexer,
  ])
  |> Util.normalize_lexers;

let rec exec_lexer = (results, buf, lex, stream) =>
  switch (lex) {
  | Lexers(ls) =>
    Lexers(
      List.fold_left(
        (acc, l) =>
          switch (exec_lexer(results, buf, l, stream)) {
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
      Util.test_match(nm, next_stream, _ =>
        t(Buffer.contents(buf)) |> Util.normalize_lexers
      )
    )
  | Result(tkn) =>
    results := [(tkn, stream), ...results^];
    None;
  }
and find_token = (results, buf, lex, stream) =>
  switch (lex, stream) {
  /* found a single longest result */
  | (Some(Result(tkn)), _) => Some((tkn, stream))
  /* found a lexer */
  | (Some(l), LazyStream.Cons((ch, _), next_stream)) =>
    Buffer.add_utf_8_uchar(buf, ch);

    switch (exec_lexer(results, buf, l, stream)) {
    /* remaining lexers */
    | Some(_) as res =>
      find_token(results, buf, res, Lazy.force(next_stream))
    /* no remaining lexers */
    | None => find_token(results, buf, None, stream)
    };
  /* ran out of lexers */
  | (None, curr) =>
    switch (results^) {
    | [x, ...xs] => Some(x)
    | [] =>
      switch (curr) {
      | LazyStream.Cons((ch, cursor), _) =>
        raise(InvalidCharacter(ch, cursor))
      | _ => None
      }
    }
  /* hit EOF */
  | (Some(l), LazyStream.Nil) =>
    switch (Util.find_result(None, l)) {
    | Some(r) => Some((r, stream))
    | None => None
    }
  };

let next_token = input => {
  let results = ref([]);

  find_token(results, Buffer.create(token_buffer_size), root, input);
};
