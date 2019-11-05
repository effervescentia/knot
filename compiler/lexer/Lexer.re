open Core;

let _token_buffer_size = 512;

let _root_matchers =
  Character.matchers
  @ Keyword.matchers
  @ Pattern.matchers
  @ Text.matchers
  @ Identifier.matchers
  @ Number.matchers
  @ Boolean.matchers
  @ Comment.matchers;

let rec find_token =
        (~buf=Buffer.create(1024), ~result=None, matchers, stream) => {
  switch (result, matchers, stream) {
  /* ran out of matchers with no result */
  | (None, [], LazyStream.Nil) => None

  /* has a final result */
  | (Some(res), [], _) => Some((res, stream))

  /* ran out of stream with remaining matchers */
  | (_, ms, LazyStream.Nil) =>
    /* check for matchers that throw errors */
    Matcher.find_error(ms) |*> throw_syntax;

    /* check for a result */
    result |?> (res => Some((res, stream)));

  /* has remaining matchers and stream */
  | (_, ms, LazyStream.Cons((c, cursor), next_stream)) =>
    Buffer.add_utf_8_uchar(buf, c);

    let match_result =
      Matcher.resolve_many(result, Buffer.contents(buf), ms, stream);

    switch (match_result) {
    /* did not match the character with no further matchers */
    | (None, []) => throw_syntax(InvalidCharacter(c, cursor))
    /* has a result or remaining matchers */
    | (next_result, next_matchers) =>
      find_token(
        ~buf,
        ~result=next_result,
        next_matchers,
        Lazy.force(next_stream),
      )
    };
  };
};

let next_token = stream => find_token(_root_matchers, stream);
