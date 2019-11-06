open Core;
open Match;
open Matcher;

let rec matchers = [
  lookahead(numeric, [Not(numeric)], s =>
    Number(int_of_string(s)) |> result
  ),
  LookaheadMatcher(numeric, [numeric], _ => many(matchers), None),
];
