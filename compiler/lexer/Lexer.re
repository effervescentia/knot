open Core;

let _token_buffer_size = 512;

let _root =
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

let rec _exec_lexer = (results, buf, lex, stream) =>
  switch (lex) {
  | Lexers(ls) =>
    let (lexers, next_results) =
      List.fold_left(
        ((acc, res), l) =>
          switch (_exec_lexer(res, buf, l, stream)) {
          | (Some(r), nres) => ([r, ...acc], nres)
          | (None, nres) => (acc, nres)
          },
        ([], results),
        ls,
      );

    (Util.normalize_lexers(Lexers(lexers)), next_results);
  | Lexer(m, nm, t) => (
      Util.test_match(m, stream, next_stream =>
        Util.test_match(nm, next_stream, _ =>
          t(Buffer.contents(buf)) |> Util.normalize_lexers
        )
      ),
      results,
    )
  | Result(tkn) => (None, [(tkn, stream), ...results])
  };

let rec _find_token = (results, buf, lexer, stream) =>
  switch (lexer, stream) {
  /* found a single longest result */
  | (Some(Result(tkn)), _) => Some((tkn, stream))

  /* found a lexer */
  | (Some(l), LazyStream.Cons((ch, _), next_stream)) =>
    Buffer.add_utf_8_uchar(buf, ch);

    switch (_exec_lexer(results, buf, l, stream)) {
    /* remaining lexers */
    | (Some(_) as res, n_results) =>
      _find_token(n_results, buf, res, Lazy.force(next_stream))

    /* no remaining lexers */
    | (None, n_results) => _find_token(n_results, buf, None, stream)
    };

  /* ran out of lexers */
  | (None, curr) =>
    switch (results) {
    /* return the longest result */
    | [x, ...xs] => Some(x)
    | [] =>
      switch (curr) {
      /* has unmatched character */
      | LazyStream.Cons((ch, cursor), _) =>
        raise(InvalidCharacter(ch, cursor))
      | _ => None
      }
    }

  /* hit EOF */
  | (Some(l), LazyStream.Nil) =>
    Util.find_result(None, l) |?> (r => Some((r, stream)))
  };

let next_token = input =>
  _find_token([], Buffer.create(_token_buffer_size), _root, input);
