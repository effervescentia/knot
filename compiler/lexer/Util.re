open Core;

let num_0_uchar = Uchar.of_char('0');
let num_9_uchar = Uchar.of_char('9');
let lowercase_a_uchar = Uchar.of_char('a');
let uppercase_a_uchar = Uchar.of_char('A');
let lowercase_z_uchar = Uchar.of_char('z');
let uppercase_z_uchar = Uchar.of_char('Z');

let is_uchar_between = ((start_ch, end_ch), ch) =>
  Uchar.compare(ch, start_ch) >= 0 && Uchar.compare(ch, end_ch) <= 0;
let is_uchar_lowercase_alpha =
  is_uchar_between((lowercase_a_uchar, lowercase_z_uchar));
let is_uchar_uppercase_alpha =
  is_uchar_between((uppercase_a_uchar, uppercase_z_uchar));
let is_uchar_alpha = ch =>
  is_uchar_lowercase_alpha(ch) || is_uchar_uppercase_alpha(ch);
let is_uchar_numeric = is_uchar_between((num_0_uchar, num_9_uchar));
let is_uchar_alphanumeric = ch => is_uchar_alpha(ch) || is_uchar_numeric(ch);

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
    switch (m) {
    | Char(ch) when Uchar.equal(Uchar.of_char(ch), c) =>
      x(Lazy.force(next_stream))
    | Alpha when is_uchar_alpha(c) => x(Lazy.force(next_stream))
    | Numeric when is_uchar_numeric(c) => x(Lazy.force(next_stream))
    | AlphaNumeric when is_uchar_alphanumeric(c) =>
      x(Lazy.force(next_stream))
    | Any => x(Lazy.force(next_stream))
    | Either(ms) =>
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
    | Except(ms) =>
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
    | Token(s) =>
      let rec next = (s, input) =>
        switch (input) {
        | LazyStream.Cons((c, _), next_input)
            when Uchar.equal(c, Uchar.of_char(s.[0])) =>
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
