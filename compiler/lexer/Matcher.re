open Knot.Core;

type t =
  | Matcher(Match.t, string => match_result, option(syntax_error))
  | LookaheadMatcher(
      Match.t,
      list(Match.t),
      string => match_result,
      option(syntax_error),
    )
and match_result = (option(token), list(t));

let result = r => (Some(r), []);

let many = ms => (None, ms);

let single = m => many([m]);

let empty = () => many([]);

let null = t => Matcher(Not(All), t, None);

let lookahead = (~error=None, match, next_matches, evaluate) =>
  LookaheadMatcher(match, next_matches, evaluate, error);

/** tokens must be a contiguous set of characters */
let rec token = (~err=None, s, t) =>
  switch (String.length(s)) {
  | 0 => null(t)
  | 1 => Matcher(Exactly(s.[0]), t, err)
  | len =>
    let substring = String.sub(s, 1, len - 1);

    LookaheadMatcher(
      Exactly(s.[0]),
      String.to_seq(substring)
      |> List.of_seq
      |> List.map(c => Match.Exactly(c)),
      _ => token(~err, substring, t) |> single,
      err,
    );
  };

/** glyphs can contain whitespace */
let glyph = (s, t) => {
  let rec next_matchers = ss =>
    if (String.length(ss) == 1) {
      let end_match = Match.Exactly(ss.[0]);
      let rec end_matchers = [
        Matcher(end_match, t, None),
        LookaheadMatcher(
          Match.whitespace,
          [Any([Match.whitespace, end_match])],
          _ => many(end_matchers),
          None,
        ),
      ];

      end_matchers;
    } else {
      let substring = String.sub(ss, 1, String.length(ss) - 1);
      let lookahead =
        String.to_seq(substring)
        |> List.of_seq
        |> List.map(c => Match.Any([Match.whitespace, Exactly(c)]));
      let rec contd_matchers = [
        LookaheadMatcher(
          Exactly(ss.[0]),
          lookahead,
          _ => next_matchers(substring) |> many,
          None,
        ),
        LookaheadMatcher(
          Match.whitespace,
          lookahead,
          _ => many(contd_matchers),
          None,
        ),
      ];

      contd_matchers;
    };

  switch (String.length(s)) {
  | 0 => null(t)
  | 1 => Matcher(Exactly(s.[0]), t, None)
  | len =>
    let substring = String.sub(s, 1, len - 1);

    LookaheadMatcher(
      Exactly(s.[0]),
      String.to_seq(substring)
      |> List.of_seq
      |> List.map(c => Match.Any([Match.whitespace, Exactly(c)])),
      _ => next_matchers(substring) |> many,
      None,
    );
  };
};

let bounded = (start, ~end_=start, t, error) => {
  let rec match_content = (boundary, _) =>
    (
      switch (String.length(boundary)) {
      | 0 => [null(t)]
      | 1 => [
          token(~err=Some(error), boundary, t),
          Matcher(
            Not(Exactly(boundary.[0])),
            match_content(boundary),
            Some(error),
          ),
        ]
      | len =>
        let substring = String.sub(boundary, 1, len - 1);
        let rec gen_matchers = (~index=1, ()) =>
          if (index == len) {
            [];
          } else {
            [
              LookaheadMatcher(
                Exactly(boundary.[0]),
                String.to_seq(substring)
                |> List.of_seq
                |> Knot.Util.slice(0, index)
                |> List.mapi((i, c) =>
                     i == index ? Match.Not(Exactly(c)) : Match.Exactly(c)
                   ),
                t,
                None,
              ),
              ...gen_matchers(~index=index + 1, ()),
            ];
          };

        [
          token(~err=Some(error), boundary, t),
          Matcher(
            Not(Exactly(boundary.[0])),
            match_content(boundary),
            None,
          ),
          ...gen_matchers(),
        ];
      }
    )
    |> many;

  token(start, match_content(end_));
};

let prefixed_line = (prefix, t) => {
  let rec match_line = _ =>
    [
      lookahead(Not(Match.end_of_line), [Match.end_of_line], t),
      lookahead(
        Not(Match.end_of_line),
        [Not(Match.end_of_line)],
        match_line,
      ),
    ]
    |> many;

  token(prefix, match_line);
};

let _exec = matcher =>
  fun
  | LazyStream.Cons((c, _), next_stream) =>
    switch (matcher) {
    /* test matcher */
    | Matcher(match, evaluate, _)
    | LookaheadMatcher(match, [], evaluate, _) when Match.test(c, match) =>
      Some(evaluate)
    /* test matcher with lookahead check */
    | LookaheadMatcher(match, next_match, evaluate, _)
        when
          Match.test(c, match)
          && Match.test_lookahead(next_match, Lazy.force(next_stream)) =>
      Some(evaluate)
    /* no matcher was successful */
    | _ => None
    }
  | LazyStream.Nil => None;

let _resolve = (curr_token, matcher, stream) =>
  _exec(matcher, stream) |?> (t => Some(t(curr_token)));

let resolve_many = (curr_result, curr_token, matchers, stream) =>
  List.fold_left(
    ((result_acc, matcher_acc), matcher) => {
      switch (_resolve(curr_token, matcher, stream)) {
      | Some((next_result, next_matchers)) => (
          next_result ||> result_acc,
          matcher_acc @ next_matchers,
        )
      | None => (result_acc, matcher_acc)
      }
    },
    (curr_result, []),
    matchers,
  );

let rec find_error =
  List.fold_left(
    (acc, matcher) =>
      acc
      ||> (
        switch (matcher) {
        | Matcher(_, _, error)
        | LookaheadMatcher(_, _, _, error) => error
        }
      ),
    None,
  );
