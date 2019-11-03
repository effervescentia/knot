open Core;

let _num_0_uchar = Uchar.of_char('0');
let _num_9_uchar = Uchar.of_char('9');
let _lowercase_a_uchar = Uchar.of_char('a');
let _uppercase_a_uchar = Uchar.of_char('A');
let _lowercase_z_uchar = Uchar.of_char('z');
let _uppercase_z_uchar = Uchar.of_char('Z');

let is_uchar_between = ((start_ch, end_ch), ch) =>
  Uchar.compare(ch, start_ch) >= 0 && Uchar.compare(ch, end_ch) <= 0;

let is_uchar_lowercase_alpha =
  is_uchar_between((_lowercase_a_uchar, _lowercase_z_uchar));

let is_uchar_uppercase_alpha =
  is_uchar_between((_uppercase_a_uchar, _uppercase_z_uchar));

let is_uchar_alpha = ch =>
  is_uchar_lowercase_alpha(ch) || is_uchar_uppercase_alpha(ch);

let is_uchar_numeric = is_uchar_between((_num_0_uchar, _num_9_uchar));

let is_uchar_alphanumeric = ch => is_uchar_alpha(ch) || is_uchar_numeric(ch);

let rec skip_whitespace = m =>
  [m, Matcher(Constants.whitespace, _ => skip_whitespace(m))]
  |> matcher_list;

let rec match_until = (curr_match, next_match, cont_match, end_match) => [
  LookaheadMatcher(Either(curr_match), Either(next_match), cont_match),
  LookaheadMatcher(Either(curr_match), Except(next_match), end_match),
];

let rec match_until_close = (end_matcher, unclosed_err, f, _) =>
  [
    Matcher(
      Except([end_matcher]),
      match_until_close(end_matcher, unclosed_err, f),
    ),
    TerminalMatcher(unclosed_err, end_matcher, s => f(s) |> result),
  ]
  |> matcher_list;

let match_bounded = (boundary_matcher, unclosed_err, f) =>
  Matcher(
    boundary_matcher,
    match_until_close(boundary_matcher, unclosed_err, f),
  );
