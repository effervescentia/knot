open Core;

include Knot.Debug;

let rec print_matcher = m =>
  switch (m) {
  | Char(c) =>
    Printf.sprintf("CHAR('%s')", String.make(1, c)) |> String.escaped
  | Token(s) => Printf.sprintf("TOKEN(%s)", s) |> String.escaped
  | Alpha => "ALPHA"
  | Numeric => "NUMERIC"
  | AlphaNumeric => "ALPHA_NUMERIC"
  | Any => "ANY"
  | Either(ms) =>
    Knot.Util.print_sequential(~separator="|", print_matcher, ms)
    |> Printf.sprintf("EITHER(%s)")
  | Except(ms) =>
    Knot.Util.print_sequential(~separator="|", print_matcher, ms)
    |> Printf.sprintf("EXCEPT(%s)")
  };

let rec print_lex_matcher =
  fun
  | Matcher(m, _)
  | TerminalMatcher(_, m, _) => Printf.sprintf("%s", print_matcher(m))
  | LookaheadMatcher(m, nm, _) =>
    Printf.sprintf("%s >> %s", print_matcher(m), print_matcher(nm));

let print_token_stream = token_stream => {
  let rec loop = stream =>
    switch (stream) {
    | LazyStream.Cons(tkn, next_stream) =>
      print_tkn(tkn) ++ loop(Lazy.force(next_stream))
    | LazyStream.Nil => ""
    };

  loop(token_stream);
};
