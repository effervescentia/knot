open Knot.Core;

type t =
  | EOF
  | Exactly(char)
  | Any(list(t))
  | Not(t)
  | All;

let _digits = int_of_char('0') <::> int_of_char('9');
let _lowercase_alpha = int_of_char('a') <::> int_of_char('z');
let _uppercase_alpha = int_of_char('A') <::> int_of_char('Z');

let numeric = Any(_digits |> List.map(d => Exactly(char_of_int(d))));

let alpha =
  Any(
    _lowercase_alpha
    @ _uppercase_alpha
    |> List.map(i => Exactly(char_of_int(i))),
  );

let alphanumeric = Any([alpha, numeric]);

let newline = Exactly('\n');
let tab = Exactly('\t');
let underscore = Exactly('_');
let space = Exactly(' ');
let quote = Exactly('"');
let dollar_sign = Exactly('$');

let whitespace = Any([space, tab, newline]);
let end_of_line = Any([newline, EOF]);

let rec test = c =>
  fun
  | Exactly(match) => Uchar.equal(c, Uchar.of_char(match))
  | Any(matches) => List.exists(match => test(c, match), matches)
  | Not(match) => !test(c, match)
  | All => true
  | EOF => false;

let rec test_lookahead = (matches, stream) =>
  switch (matches) {
  /* no matches remaining */
  | [] => true
  /* has matches remaining */
  | [match, ...ms] =>
    switch (match, stream) {
    /* empty match set, always false */
    | (Not(All), _) => false
    /* test lookahead with current character */
    | (_, LazyStream.Cons((c, _), next_stream)) =>
      test(c, match) && test_lookahead(ms, Lazy.force(next_stream))
    /* can match EOF */
    | (EOF | All | Not(_), LazyStream.Nil) when List.length(ms) == 0 => true
    /* does not match */
    | _ => false
    }
  };
