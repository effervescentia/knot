open Knot.Core;

type t =
  | Matcher(Match.t, evaluator)
  | LookaheadMatcher(Match.t, list(Match.t), evaluator)
  | BoundaryError(Match.t, ((int, int)) => syntax_error)
and evaluator = (unit => string) => (option(token), list(t));

type uchar_stream = LazyStream.t((Uchar.t, (int, int)));

type active_matcher = {
  matcher: t,
  initial: uchar_stream,
  current: uchar_stream,
  length: int,
};

let _initial_token_buffer_size = 128;

/**
 * create a string representation of the token
 */
let rec _generate_token =
        (~buf=Buffer.create(_initial_token_buffer_size), stream) =>
  fun
  | 0 => Buffer.contents(buf)
  | len =>
    switch (stream) {
    /* add unicode characters to the buffer */
    | LazyStream.Cons((c, _), next_stream) =>
      Buffer.add_utf_8_uchar(buf, c);

      _generate_token(~buf, Lazy.force(next_stream), len - 1);
    /* unable to complete the token */
    | LazyStream.Nil => invariant(CannotGenerateToken)
    };

let resolve = result =>
  fun
  /* match against the current character in the stream */
  | {
      matcher: Matcher(match, t) | LookaheadMatcher(match, [], t),
      initial: LazyStream.Cons((_, cursor), _) as initial,
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
          next_result
          |?> (token => Some(({token, cursor, length}, next_stream)))
          ||> result,
          next_matchers
          |> List.map(matcher =>
               {matcher, initial, current: next_stream, length: length + 1}
             ),
        );
      }
    )

  /* match the current and all lookahead characters against the stream */
  | {
      matcher: LookaheadMatcher(match, lookahead, t),
      initial: LazyStream.Cons((_, cursor), _) as initial,
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
        |?> (
          token => Some(({token, cursor, length}, Lazy.force(next_stream)))
        )
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

  /* match boundary errors */
  | {
      matcher: BoundaryError(match, err),
      initial: LazyStream.Cons((_, cursor), _),
      current: stream,
    }
      when Match.test_match_stream(stream, match) =>
    throw_syntax(err(cursor))

  /* no matches */
  | _ => (result, []);
