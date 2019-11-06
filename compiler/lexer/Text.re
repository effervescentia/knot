open Core;
open Matcher;

let _quote = Match.Exactly('"');

let rec _match_contd = (t, _) =>
  [
    Matcher(_quote, t, Some(UnclosedString)),
    Matcher(Not(_quote), _match_contd(t), Some(UnclosedString)),
  ]
  |> many;

let matchers = [
  Matcher(
    _quote,
    _match_contd(s =>
      String(String.sub(s, 1, String.length(s) - 2)) |> result
    ),
    None,
  ),
];
