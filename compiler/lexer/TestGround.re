open Knot.Core;

type match_pattern =
  | Char(char)
  | Any(list(match_pattern))
  | Not(match_pattern)
  | All;

type matcher('a, 'b) =
  | Matcher(match_pattern, string => match_result('a, 'b))
  | LookaheadMatcher(
      match_pattern,
      match_pattern,
      string => match_result('a, 'b),
    )
  | TerminalMatcher(exn, match_pattern, string => match_result('a, 'b))
and match_result('a, 'b) = (option('b), list(matcher('a, 'b)));

module Matcher = {
  let _digits = int_of_char('0') <::> int_of_char('9');
  let _lowercase_alpha = int_of_char('a') <::> int_of_char('z');
  let _uppercase_alpha = int_of_char('A') <::> int_of_char('Z');

  let result = r => (Some(r), []);

  let many = ms => (None, ms);

  let single = m => many([m]);

  let empty = () => many([]);

  let numeric = Any(_digits |> List.map(d => Char(char_of_int(d))));

  let alpha =
    Any(
      _lowercase_alpha
      @ _uppercase_alpha
      |> List.map(i => Char(char_of_int(i))),
    );

  let alphanumeric = Any([alpha, numeric]);

  let rec token = (s, res) =>
    if (String.length(s) === 1) {
      Matcher(Char(s.[0]), _ => res |> result);
    } else {
      Matcher(
        Char(s.[0]),
        _ => token(String.sub(s, 1, String.length(s) - 1), res) |> single,
      );
    };
};

let rec number_matchers = [
  LookaheadMatcher(
    Matcher.numeric,
    Not(Matcher.numeric),
    s => Number(int_of_string(s)) |> Matcher.result,
  ),
  LookaheadMatcher(
    Matcher.numeric,
    Matcher.numeric,
    _ => number_matchers |> Matcher.many,
  ),
];

let rec test_match = c =>
  fun
  | Char(match) => {
      // Printf.sprintf(
      //   "comparing %s to %s (%b)",
      //   Knot.Util.print_uchar(c),
      //   String.make(1, match),
      //   Uchar.equal(c, Uchar.of_char(match)),
      // )
      // |> print_endline;
      Uchar.equal(
        c,
        Uchar.of_char(match),
      );
    }
  | Any(matches) => {
      // print_string("any of: ");
      List.exists(
        match => test_match(c, match),
        matches,
      );
    }
  | Not(match) => {
      !test_match(c, match);
    }
  | All => true;

let test_lookahead_match = (match, stream) =>
  switch (match, stream) {
  | (_, LazyStream.Cons((c, _), _)) => test_match(c, match)
  | (All | Not(_), LazyStream.Nil) => true
  | (_, LazyStream.Nil) => false
  };

let exec_matcher = matcher =>
  fun
  | LazyStream.Cons((c, _), next_stream) =>
    switch (matcher) {
    | Matcher(match, res)
    | TerminalMatcher(_, match, res) when test_match(c, match) => Some(res)
    | LookaheadMatcher(match, next_match, res)
        when
          test_match(c, match)
          && test_lookahead_match(next_match, Lazy.force(next_stream)) =>
      // Printf.sprintf("match successful!") |> print_endline;
      Some(res)
    | _ => None
    }
  | LazyStream.Nil => None;

let resolve_matcher = (curr_token, matcher, stream) => {
  switch (exec_matcher(matcher, stream)) {
  | Some(t) => Some(t(curr_token))
  | None => None
  };
};

let resolve_matchers = (curr_result, curr_token, matchers, stream) =>
  List.fold_left(
    ((result_acc, matcher_acc), matcher) => {
      // print_endline("resolve more");
      switch (resolve_matcher(curr_token, matcher, stream)) {
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

let rec find_token =
        (~buf=Buffer.create(1024), ~result=None, matchers, stream) => {
  switch (result, matchers, stream) {
  /* has a final result */
  | (Some(res), [], _) => Some((res, stream))
  /* has remaining matchers and stream */
  | (None, ms, LazyStream.Cons((c, _), next_stream))
      when List.length(ms) !== 0 =>
    Buffer.add_utf_8_uchar(buf, c);

    // Printf.sprintf("found %s", Buffer.contents(buf)) |> print_endline;

    let (next_result, next_matchers) =
      resolve_matchers(result, Buffer.contents(buf), ms, stream);

    // Knot.Util.print_optional(
    //   Debug.print_tkn % Printf.sprintf("RESULT %s"),
    //   next_result,
    // )
    // |> print_endline;

    // Printf.sprintf("length %d", List.length(next_matchers)) |> print_endline;

    find_token(
      ~buf,
      ~result=next_result,
      next_matchers,
      Lazy.force(next_stream),
    );
  | _ => None
  };
};

let next_token = stream => find_token(number_matchers, stream);
