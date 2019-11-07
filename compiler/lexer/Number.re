open Core;
open Match;
open Matcher;

let rec matchers = [
  LookaheadMatcher(
    numeric,
    [Not(numeric)],
    get_string => Number(int_of_string(get_string())) |> result,
  ),
  LookaheadMatcher(numeric, [numeric], _ => many(matchers)),
];
