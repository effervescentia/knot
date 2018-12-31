open Core;
open Knot.Token;

module LazyStream = Knot.LazyStream;

let root =
  Lexers([
    Character.lexer,
    Keyword.lexer,
    Pattern.lexer,
    Text.lexer,
    Identifier.lexer,
    Number.lexer,
    Comment.lexer,
  ])
  |> Util.normalize_lexers;

let next_token = input => {
  let results = ref([]);

  let rec exec_lexer = (s, lex, stream) =>
    switch (lex) {
    | Lexers(ls) =>
      Lexers(
        List.fold_left(
          (acc, l) =>
            switch (exec_lexer(s, l, stream)) {
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
  and find_token = (s, lex, stream) =>
    switch (lex, stream) {
    /* found a single longest result */
    | (Some(Result(tkn)), _) => Some((tkn, stream))
    /* found a lexer */
    | (Some(l), LazyStream.Cons((ch, _), next_stream)) =>
      let next_string = s ++ String.make(1, ch);

      switch (exec_lexer(next_string, l, stream)) {
      /* remaining lexers */
      | Some(_) as res =>
        find_token(next_string, res, Lazy.force(next_stream))
      /* no remaining lexers */
      | None => find_token(s, None, stream)
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
  find_token("", root, input);
};