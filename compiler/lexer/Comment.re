open Core;

let matchers =
  Matcher.[
    bounded(
      "///",
      // s =>
      //   String.length(s) >= 6
      //     ? BlockComment(String.sub(s, 3, String.length(s) - 6)) |> result
      //     : Matcher.empty(),
      s => BlockComment(String.sub(s, 3, String.length(s) - 6)) |> result,
      UnclosedCommentBlock((2, 2)),
    ),
    lookahead(Exactly('/'), [Exactly('/'), Match.end_of_line], _ =>
      LineComment("") |> result
    ),
    prefixed_line("//", s =>
      LineComment(String.sub(s, 2, String.length(s) - 2)) |> result
    ),
  ];
