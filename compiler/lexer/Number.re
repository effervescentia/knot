open Core;

let rec matchers =
  Matcher.[
    lookahead(Match.numeric, [Not(Match.numeric)], s =>
      Number(int_of_string(s)) |> result
    ),
    LookaheadMatcher(
      Match.numeric,
      [Match.numeric],
      _ => many(matchers),
      None,
    ),
  ];
