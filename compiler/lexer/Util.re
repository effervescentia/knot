open Core;
open Match;
open Matcher;

let rec match_until_eol = (t, _) =>
  [
    LookaheadMatcher(All, [end_of_line], t),
    LookaheadMatcher(All, [Not(end_of_line)], match_until_eol(t)),
  ]
  |> many;

let rec match_until = (t, s, err, _) =>
  switch (String.length(s)) {
  | 0 => invariant(InvalidToken)
  | len =>
    let match_contd =
      Matcher(Not(Exactly(s.[0])), match_until(t, s, err) % many);

    [
      Matcher(
        Exactly(s.[0]),
        len == 1
          ? t
          : match_until(t, String.sub(s, 1, len - 1), err)
            % (matchers => matchers @ [match_contd] |> many),
      ),
      match_contd,
      BoundaryError(EOF, err),
    ];
  };

let rec match_until_char = (match, err, t, _) =>
  [
    Matcher(match, t),
    Matcher(Not(match), match_until_char(match, err, t)),
    BoundaryError(EOF, err),
  ]
  |> many;

let match_bounded = (boundary, err, t) =>
  token(boundary, match_until(t, boundary, err) % many);

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

let rec match_while = (match, t, _) =>
  [
    LookaheadMatcher(match, [Not(match)], t),
    LookaheadMatcher(match, [match], match_while(match, t)),
  ]
  |> many;
