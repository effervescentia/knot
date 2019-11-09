open Core;
open Match;
open Matcher;

let _comment_start = "/*";
let _comment_end = "*/";

let _prefixed_line_matcher = t =>
  Matcher(
    forward_slash,
    _ =>
      [
        LookaheadMatcher(forward_slash, [end_of_line], t),
        LookaheadMatcher(
          forward_slash,
          [Not(end_of_line)],
          Util.match_until_eol(t),
        ),
      ]
      |> many,
  );

let matchers = [
  Util.match_bounded(
    _comment_start,
    _comment_end,
    cursor => UnclosedCommentBlock(cursor),
    get_string => {
      let string = get_string();

      BlockComment(String.sub(string, 2, String.length(string) - 4))
      |> result;
    },
  ),
  _prefixed_line_matcher(get_string => {
    let string = get_string();
    let comment = String.sub(string, 2, String.length(string) - 2);

    LineComment(comment) |> result;
  }),
];
