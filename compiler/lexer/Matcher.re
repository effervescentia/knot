open Knot.Core;
open Match;

type t =
  | Matcher(Match.t, string => match_result, option(syntax_error))
  | LookaheadMatcher(
      Match.t,
      list(Match.t),
      string => match_result,
      option(syntax_error),
    )
  | PermissiveLookaheadMatcher(
      Match.t,
      Match.t,
      list(Match.t),
      string => match_result,
    )
and match_result = (option(token), list(t));

let result = r => (Some(r), []);

let many = ms => (None, ms);

let single = m => many([m]);

let empty = () => many([]);

let null = t => Matcher(Not(All), t, None);

let lookahead = (~error=None, match, next_matches, evaluate) =>
  LookaheadMatcher(match, next_matches, evaluate, error);

let _exec = matcher =>
  fun
  | LazyStream.Cons((c, _), next_stream) =>
    switch (matcher) {
    /* test matcher */
    | Matcher(match, evaluate, _)
    | LookaheadMatcher(match, [], evaluate, _)
    | PermissiveLookaheadMatcher(match, _, [], evaluate)
        when Match.test(c, match) =>
      Some(evaluate)
    /* test matcher with lookahead check */
    | LookaheadMatcher(match, next_match, evaluate, _)
        when
          Match.test(c, match)
          && Match.test_lookahead(next_match, Lazy.force(next_stream)) =>
      Some(evaluate)
    /* test matcher with permissive lookahead check */
    | PermissiveLookaheadMatcher(match, skip_match, next_match, evaluate)
        when
          Match.test(c, match)
          && Match.test_lookahead(
               ~skip_match,
               next_match,
               Lazy.force(next_stream),
             ) =>
      Some(evaluate)
    /* no matcher was successful */
    | _ => None
    }

  | LazyStream.Nil =>
    switch (matcher) {
    | Matcher(match, evaluate, _)
    | LookaheadMatcher(match, [], evaluate, _) when Match.test_eof(match) =>
      Some(evaluate)
    | _ => None
    };

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
        | PermissiveLookaheadMatcher(_) => None
        }
      ),
    None,
  );
