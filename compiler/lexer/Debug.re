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
    Printf.sprintf(
      "EITHER(%s)",
      List.fold_left((acc, m) => acc ++ "|" ++ print_matcher(m), "", ms),
    )
  | Except(ms) =>
    Printf.sprintf(
      "EXCEPT(%s)",
      List.fold_left((acc, m) => acc ++ "|" ++ print_matcher(m), "", ms),
    )
  };

let rec print_lex_table = l =>
  switch (l) {
  | Lexers(ls) =>
    List.fold_left((acc, l) => acc ++ print_lex_table(l), "", ls)
  | Lexer(m, nm, _)
  | FailingLexer(_, m, nm, _) =>
    Printf.sprintf("%s >> %s\n", print_matcher(m), print_matcher(nm))
  | Result(x) => "RESULT!\n"
  };

let print_token_stream = token_stream => {
  let rec loop = stream =>
    switch (stream) {
    | LazyStream.Cons(tkn, next_stream) =>
      print_tkn(tkn) ++ loop(Lazy.force(next_stream))
    | LazyStream.Nil => ""
    };

  loop(token_stream);
};
