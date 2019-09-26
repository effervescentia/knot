open Core;

let _num_0_uchar = Uchar.of_char('0');
let _num_9_uchar = Uchar.of_char('9');
let _lowercase_a_uchar = Uchar.of_char('a');
let _uppercase_a_uchar = Uchar.of_char('A');
let _lowercase_z_uchar = Uchar.of_char('z');
let _uppercase_z_uchar = Uchar.of_char('Z');

let is_uchar_between = ((start_ch, end_ch), ch) =>
  Uchar.compare(ch, start_ch) >= 0 && Uchar.compare(ch, end_ch) <= 0;
let is_uchar_lowercase_alpha =
  is_uchar_between((_lowercase_a_uchar, _lowercase_z_uchar));
let is_uchar_uppercase_alpha =
  is_uchar_between((_uppercase_a_uchar, _uppercase_z_uchar));
let is_uchar_alpha = ch =>
  is_uchar_lowercase_alpha(ch) || is_uchar_uppercase_alpha(ch);
let is_uchar_numeric = is_uchar_between((_num_0_uchar, _num_9_uchar));
let is_uchar_alphanumeric = ch => is_uchar_alpha(ch) || is_uchar_numeric(ch);

let flatten_lexers =
  fun
  | [] => None
  | [x] => Some(x)
  | ls => Some(Lexers(ls));

let rec expand_lexer = (matchers, tag, tag_lexer, s) =>
  if (String.length(s) == 1) {
    tag_lexer(Char(s.[0]), matchers, tag);
  } else {
    tag_lexer(Char(s.[0]), Char(s.[1]), _ =>
      expand_lexer(
        matchers,
        tag,
        tag_lexer,
        String.sub(s, 1, String.length(s) - 1),
      )
    );
  };

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
  | Lexer(Token(tkn_str), nm, t) =>
    Some(expand_lexer(nm, t, (m1, m2, tag) => Lexer(m1, m2, tag), tkn_str))
  /* expand token matchers */
  | FailingLexer(e, Token(tkn_str), nm, t) =>
    Some(
      expand_lexer(
        nm,
        t,
        (m1, m2, tag) => FailingLexer(e, m1, m2, tag),
        tkn_str,
      ),
    )
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
    | Either(ms) => _first_match(stream, x, ms)
    | Except(ms) =>
      _has_matches(stream, x, ms) ? x(Lazy.force(next_stream)) : None
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
  }
and _first_match = (stream, x) =>
  fun
  | [m, ...ms] =>
    test_match(m, stream, x) |=> (() => _first_match(stream, x, ms))
  | [] => None
and _has_matches = (stream, x) =>
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
  );

let rec find_error = r =>
  fun
  | Lexers(ls) =>
    List.fold_left((acc, l) => acc |=> (() => find_error(r, l)), None, ls)
  | FailingLexer(e, _, _, _) => Some(e)
  | _ => None;

let rec find_result = r =>
  fun
  | Lexers(ls) =>
    List.fold_left((acc, l) => acc |=> (() => find_result(r, l)), None, ls)
  | Result(res) => Some(res)
  | _ => None;
