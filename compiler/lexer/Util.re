open Globals;
open Match;
open Matcher;

let rec match_until_eol = (t, _) =>
  [
    LookaheadMatcher(All, [end_of_line], t),
    LookaheadMatcher(All, [Not(end_of_line)], match_until_eol(t)),
  ]
  |> many;

let rec match_until_char = (match, err, t) => [
  Matcher(match, t),
  Matcher(Not(match), _ => match_until_char(match, err, t) |> many),
  BoundaryError(EOF, err),
];

let rec match_until = (t, s, err, _) =>
  switch (String.length(s)) {
  | 0 => invariant(InvalidToken)
  | len =>
    let rec match_rest = ss => [
      // continue the match
      Matcher(
        Exactly(ss.[0]),
        String.length(ss) == 1
          ? t
          : (
            _ =>
              match_rest(String.sub(ss, 1, String.length(ss) - 1)) |> many
          ),
      ),
      // restart the match
      Matcher(Not(Exactly(ss.[0])), match_until(t, s, err) % many),
      BoundaryError(EOF, err),
    ];

    match_until_char(Exactly(s.[0]), err, _ =>
      match_rest(String.sub(s, 1, len - 1)) |> many
    );
  };

let match_bounded = (start, end_, err, t) =>
  token(start, match_until(t, end_, err) % many);

let rec match_tentative = t =>
  fun
  | [char, ...next_chars] as chars => (
      _ =>
        [
          Matcher(
            Exactly(char),
            List.length(next_chars) == 0
              ? t : match_tentative(t, next_chars),
          ),
          Matcher(whitespace, match_tentative(t, chars)),
        ]
        |> many
    )
  | _ => (_ => empty);

let rec match_while = (match, t) => [
  LookaheadMatcher(match, [Not(match)], t),
  LookaheadMatcher(match, [match], _ => match_while(match, t) |> many),
];
