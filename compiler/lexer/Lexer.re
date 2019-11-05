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
  /* ran out of stream with ongoing matches */
  | (_, ms, LazyStream.Nil) when List.length(ms) !== 0 =>
    Matcher.find_error(ms) |*> throw_syntax;

    switch (result) {
    | Some(res) => Some((res, stream))
    | None => None
    };

  /* has a final result */
  | (Some(res), [], _)
  | (Some(res), _, LazyStream.Nil) => Some((res, stream))

  /* has remaining matchers and stream */
  | (_, ms, LazyStream.Cons((c, cursor), next_stream))
      when List.length(ms) !== 0 =>
    Buffer.add_utf_8_uchar(buf, c);

    let match_result =
      Matcher.resolve_many(result, Buffer.contents(buf), ms, stream);

    switch (match_result) {
    | (None, []) => throw_syntax(InvalidCharacter(c, cursor))
    | (next_result, next_matchers) =>
      find_token(
        ~buf,
        ~result=next_result,
        next_matchers,
        Lazy.force(next_stream),
      )
    };

  | _ => None
  };
};

let next_token = stream => find_token(_root_matchers, stream);
