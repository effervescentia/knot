open Core;

let rec match_comment_block = _ =>
  [
    Matcher(Except([Constants.triple_slash]), match_comment_block),
    TerminalMatcher(
      UnclosedCommentBlock((2, 2)),
      Constants.triple_slash,
      s => result(BlockComment(String.sub(s, 3, String.length(s) - 6))),
    ),
  ]
  |> matcher_list;

let rec match_comment_line = _ =>
  [
    Matcher(Except([Constants.newline]), match_comment_line),
    LookaheadMatcher(
      Except([Constants.newline]),
      Constants.newline,
      s => result(LineComment(String.sub(s, 2, String.length(s) - 2))),
    ),
  ]
  |> matcher_list;

let matchers = [
  Matcher(Constants.triple_slash, match_comment_block),
  Matcher(Constants.double_slash, match_comment_line),
];
