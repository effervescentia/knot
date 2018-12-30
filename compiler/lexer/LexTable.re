open Knot.Token;
open Core;

module LazyStream = Knot.LazyStream;

exception LexTableInvalid;

let flatten_lexers = ls =>
  switch (ls) {
  | [] => None
  | [x] => Some(x)
  | _ => Some(Lexers(ls))
  };

let rec normalize_lexers = lex =>
  switch (lex) {
  | Lexers(ls) =>
    List.fold_left(
      (acc, l) =>
        switch (normalize_lexers(l)) {
        | Some(Lexers(xs)) => List.append(xs, acc)
        | Some(l) => [l, ...acc]
        | None => acc
        },
      [],
      ls,
    )
    |> flatten_lexers
  | Lexer(Either(ms), nm, t) =>
    Some(Lexers(List.map(m => Lexer(m, nm, t), ms)))
  | Lexer(Token(s), nm, t) =>
    let rec next = s =>
      if (String.length(s) == 1) {
        Lexer(Char(s.[0]), nm, t);
      } else {
        Lexer(
          Char(s.[0]),
          Char(s.[1]),
          lazy (next(String.sub(s, 1, String.length(s) - 1))),
        );
      };
    Some(next(s));
  | Lexer(_, _, _) as l => Some(l)
  | _ as res => Some(res)
  };

let lexer =
  Lexers([
    Character.lexer,
    Keyword.lexer,
    Pattern.lexer,
    Text.lexer,
    Identifier.lexer,
    Number.lexer,
    Comment.lexer,
  ])
  |> normalize_lexers;

let rec compute_match = (m, stream, x) =>
  switch (stream) {
  | LazyStream.Cons((c, _), next_stream) =>
    switch (m, c) {
    | (Char(ch), _) when ch == c => x(Lazy.force(next_stream))
    | (Alpha, 'a'..'z' | 'A'..'Z')
    | (Numeric, '0'..'9')
    | (AlphaNumeric, 'a'..'z' | 'A'..'Z' | '0'..'9')
    | (Any, _) => x(Lazy.force(next_stream))
    | (Either(ms), _) =>
      let rec first_match = (
        fun
        | [m, ...ms] =>
          switch (compute_match(m, stream, x)) {
          | Some(_) as res => res
          | None => first_match(ms)
          }
        | [] => None
      );

      first_match(ms);
    | (Except(ms), _) =>
      let has_matches =
        List.fold_left(
          (acc, m) =>
            acc
            && (
              switch (compute_match(m, stream, x)) {
              | Some(v) => false
              | None => true
              }
            ),
          true,
          ms,
        );

      if (has_matches) {
        x(Lazy.force(next_stream));
      } else {
        None;
      };
    /* | (Token(s), _) =>  */
    | _ => None
    }
  | LazyStream.Nil => None
  };

let next_token = input => {
  let results = ref([]);

  let rec next = (s, lex, stream) =>
    switch (lex) {
    | Lexers(ls) =>
      Some(
        Lexers(
          List.fold_left(
            (acc, l) =>
              switch (next(s, l, stream)) {
              | Some(r) => [r, ...acc]
              | None => acc
              },
            [],
            ls,
          ),
        ),
      )
    | Lexer(m, nm, t) =>
      compute_match(m, stream, next_stream =>
        compute_match(nm, next_stream, _ => Some(Lazy.force(t)))
      )
    | Result(r) =>
      results := [(r(s), stream), ...results^];
      None;
    }
  and find_token = (s, lex, stream) => {
    print_endline(
      Printf.sprintf("looking for token, matched so far: '%s'", s),
    );

    switch (lex, stream) {
    | (Some(Result(r)), _) =>
      let tkn = r(s);
      print_endline(
        Printf.sprintf("found result: '%s'", Debug.print_tkn(tkn)),
      );

      Some((tkn, stream));
    | (Some(l), LazyStream.Cons((ch, _), next_stream)) =>
      let next_string = s ++ String.make(1, ch);
      print_endline("checking for matching lexers");

      switch (next(next_string, l, stream)) {
      | Some(lx) =>
        print_endline("found matching lexers");

        find_token(
          next_string,
          lx |> normalize_lexers,
          Lazy.force(next_stream),
        );
      | None => find_token(next_string, None, Lazy.force(next_stream))
      };
    | (None, _) =>
      print_endline(
        Printf.sprintf(
          "out of lexers with %d results",
          List.length(results^),
        ),
      );

      switch (results^) {
      | [x, ...xs] => Some(x)
      | [] => None
      };
    | (_, LazyStream.Nil) =>
      print_endline("stream is empty");

      None;
    };
  };

  find_token("", lexer, input);
};