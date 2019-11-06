open Core;
open Match;
open Matcher;

let _double_slash = "//";
let _triple_slash = "///";

let rec prefixed = (s, t) => {
  let rec await_eol = _ =>
    [
      LookaheadMatcher(All, [Match.end_of_line], t, None),
      LookaheadMatcher(All, [Not(Match.end_of_line)], await_eol, None),
    ]
    |> many;

  switch (String.length(s)) {
  | 0 => [null(t)]
  | 1 => [
      LookaheadMatcher(Exactly(s.[0]), [Match.end_of_line], t, None),
      LookaheadMatcher(
        Exactly(s.[0]),
        /* special case to avoid capturing comment blocks */
        [Not(Any([Match.end_of_line, Exactly('/')]))],
        await_eol,
        None,
      ),
    ]

  | len =>
    let substring = String.sub(s, 1, len - 1);

    [
      LookaheadMatcher(
        Exactly(s.[0]),
        [Exactly(s.[1])],
        _ => prefixed(substring, t) |> many,
        None,
      ),
    ];
  };
};

let rec match_until = (s, t, err, _) => {
  let rec match_contd = (ss, _) =>
    [
      Matcher(
        Exactly(ss.[0]),
        String.length(ss) == 1
          ? t : String.sub(ss, 1, String.length(ss) - 1) |> match_contd,
        err,
      ),
      Matcher(Not(Exactly(ss.[0])), match_until(s, t, err), err),
    ]
    |> many;

  [
    Matcher(
      Exactly(s.[0]),
      match_contd(String.sub(s, 1, String.length(s) - 1)),
      err,
    ),
    Matcher(Not(Exactly(s.[0])), match_until(s, t, err), err),
  ]
  |> many;
};

let match_bounded = (boundary, t, err) =>
  token("///", match_until("///", t, Some(err)));

let matchers = [
  match_bounded(
    "///",
    s =>
      BlockComment(
        String.sub(
          s,
          String.length(_triple_slash),
          String.length(s) - String.length(_triple_slash) * 2,
        ),
      )
      |> result,
    UnclosedCommentBlock((2, 2)),
  ),
  ...prefixed(_double_slash, s =>
       LineComment(
         String.sub(
           s,
           String.length(_double_slash),
           String.length(s) - String.length(_double_slash),
         ),
       )
       |> result
     ),
];
