open Knot.Core;

type t =
  | Exactly(char)
  | Not(t)
  | Any(list(t))
  | All
  | EOF;

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
let forward_slash = Exactly('/');

let whitespace = Any([space, tab, newline]);
let end_of_line = Any([newline, EOF]);

let rec test_eof =
  fun
  | EOF => true
  | Not(match) => !test_eof(match)
  | Any(matches) => List.exists(match => test_eof(match), matches)
  | _ => false;

let rec test_match = c =>
  fun
  | Exactly(ch) => Uchar.of_char(ch) |> Uchar.equal(c)
  | Not(match) => !test_match(c, match)
  | Any(matches) => List.exists(match => test_match(c, match), matches)
  | All => true
  | EOF => false;

let test_match_stream =
  fun
  | LazyStream.Nil => test_eof
  | LazyStream.Cons((c, _), _) => test_match(c);

let rec test_lookahead = (matches, stream) =>
  switch (matches, stream) {
  | ([], _) => true
  | ([match], stream) => test_match_stream(stream, match)
  | ([match, ...next_matches], LazyStream.Cons(_, next_stream) as stream) =>
    test_match_stream(stream, match)
    && test_lookahead(next_matches, Lazy.force(next_stream))
  | (_, LazyStream.Nil) => false
  };
