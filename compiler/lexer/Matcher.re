open Knot.Core;

type t =
  | Matcher(Match.t, evaluator)
  | LookaheadMatcher(Match.t, list(Match.t), evaluator)
  | BoundaryError(Match.t, cursor => syntax_error)
and evaluator = (unit => string) => (option(token), list(t));

type active_matcher = {
  matcher: t,
  initial: uchar_stream,
  current: uchar_stream,
  length: int,
};

let _initial_token_buffer_size = 128;

let rec _generate_token =
        (~buf=Buffer.create(_initial_token_buffer_size), stream) =>
  fun
  | 0 => Buffer.contents(buf)
  | len =>
    switch (stream) {
    | LazyStream.Cons((c, _), next_stream) =>
      Buffer.add_utf_8_uchar(buf, c);

      _generate_token(~buf, Lazy.force(next_stream), len - 1);
    | LazyStream.Nil => invariant(CannotGenerateToken)
    };

let resolve = result =>
  fun
  | {
      matcher: Matcher(match, t) | LookaheadMatcher(match, [], t),
      initial,
      length,
      current: stream,
    }
      when Match.test_match_stream(stream, match) =>
    (
      switch (stream) {
      | LazyStream.Cons(_, next_stream) => Lazy.force(next_stream)
      | _ => stream
      }
    )
    |> (
      next_stream => {
        let (next_result, next_matchers) =
          t(() => _generate_token(initial, length));

        (
          next_result |?> (res => Some((res, next_stream))) ||> result,
          next_matchers
          |> List.map(matcher =>
               {matcher, initial, current: next_stream, length: length + 1}
             ),
        );
      }
    )

  | {
      matcher: LookaheadMatcher(match, lookahead, t),
      initial,
      length,
      current: LazyStream.Cons((c, _), next_stream),
    }
      when
        Match.test_match(c, match)
        && Match.test_lookahead(lookahead, Lazy.force(next_stream)) => {
      let (next_result, next_matchers) =
        t(() => _generate_token(initial, length));

      (
        next_result
        |?> (res => Some((res, Lazy.force(next_stream))))
        ||> result,
        next_matchers
        |> List.map(matcher =>
             {
               matcher,
               initial,
               current: Lazy.force(next_stream),
               length: length + 1,
             }
           ),
      );
    }

  | {
      matcher: BoundaryError(match, err),
      initial: LazyStream.Cons((_, cursor), _),
      current: stream,
    }
      when Match.test_match_stream(stream, match) =>
    throw_syntax(err(cursor))

  | _ => (result, []);
