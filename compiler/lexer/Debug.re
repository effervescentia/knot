open Core;
open Match;
open Matcher;

include KnotDebug.Debug;

let rec print_match = m =>
  switch (m) {
  | Exactly(c) =>
    Printf.sprintf("CHAR('%s')", String.make(1, c)) |> String.escaped
  | Any(ms) =>
    Knot.Util.print_sequential(~separator="|", print_match, ms)
    |> Printf.sprintf("ANY(%s)")
  | Not(m) => print_match(m) |> Printf.sprintf("NOT(%s)")
  | All => "ALL"
  | EOF => "EOF"
  };

let rec print_matcher =
  fun
  | Matcher(m, _)
  | LookaheadMatcher(m, [], _) => print_match(m)
  | LookaheadMatcher(m, [nm, ...nms], evaluate) =>
    Printf.sprintf(
      "%s >> %s",
      print_match(m),
      LookaheadMatcher(nm, nms, evaluate) |> print_matcher,
    )
  | BoundaryError(_) => "boundary error";

let print_token_stream = token_stream => {
  let rec loop = stream =>
    switch (stream) {
    | LazyStream.Cons(tkn, next_stream) =>
      print_tkn(tkn) ++ loop(Lazy.force(next_stream))
    | LazyStream.Nil => ""
    };

  loop(token_stream);
};
