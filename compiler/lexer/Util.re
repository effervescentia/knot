open Core;

let rec (===>) = (s, t) => {
  let next = _ =>
    if (String.length(s) == 1) {
      Result(t);
    } else {
      String.sub(s, 1, String.length(s) - 1) ===> t;
    };

  Lexer(Char(s.[0]), Any, next);
};

let flatten_lexers =
  fun
  | [] => None
  | [x] => Some(x)
  | ls => Some(Lexers(ls));

let rec normalize_lexers =
  fun
  /* flatten nested lexers */
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
  /* expand token matchers */
  | Lexer(Token(s), nm, t) => {
      let rec next = s =>
        if (String.length(s) == 1) {
          Lexer(Char(s.[0]), nm, t);
        } else {
          Lexer(
            Char(s.[0]),
            Char(s.[1]),
            (_ => next(String.sub(s, 1, String.length(s) - 1))),
          );
        };
      Some(next(s));
    }
  | _ as res => Some(res);

let rec test_match = (m, stream, x) =>
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
          switch (test_match(m, stream, x)) {
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
              switch (test_match(m, stream, x)) {
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
    | (Token(s), _) =>
      let rec next = (s, input) =>
        switch (input) {
        | LazyStream.Cons((c, _), next_input) when c == s.[0] =>
          if (String.length(s) == 1) {
            x(Lazy.force(next_input));
          } else {
            next(
              String.sub(s, 1, String.length(s) - 1),
              Lazy.force(next_input),
            );
          }
        | _ => None
        };

      next(s, stream);
    | _ => None
    }
  | LazyStream.Nil =>
    switch (m) {
    | Except(_)
    | Any => x(stream)
    | _ => None
    }
  };

let rec find_result = r =>
  fun
  | Lexers(ls) =>
    List.fold_left(
      (acc, l) =>
        switch (acc) {
        | Some(_) as res => res
        | None => find_result(r, l)
        },
      None,
      ls,
    )
  | Result(res) => Some(res)
  | Lexer(_, _, _) => None;