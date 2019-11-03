open Core;

let _token_buffer_size = 512;

/**
 * 1. have a single stream to keep track of
 * 2. take a list of matchers and either return a result or a list of matchers for the next char
 */

let _root_matchers =
  Character.matchers
  @ Keyword.matchers
  @ Pattern.matchers
  @ Text.matchers
  // @ Identifier.matchers
  @ Number.matchers
  @ Boolean.matchers
  @ Comment.matchers;

let rec _find_token =
        (
          ~result=None,
          ~buf=Buffer.create(_token_buffer_size),
          matchers,
          stream,
        ) =>
  if (List.length(matchers) == 0) {
    print_endline("RAN OUT OF LEXERS");

    switch (result, stream) {
    /* result found */
    | (Some(tkn), _) =>
      Printf.sprintf(
        "RESULT: %s",
        Knot.Util.print_optional(Debug.print_tkn, result),
      )
      |> print_endline;
      switch (stream) {
      | LazyStream.Cons((x, _), _) =>
        Knot.Util.print_uchar(x)
        |> Printf.sprintf("NEXT STREAM CHAR: %s")
        |> print_endline
      | LazyStream.Nil => print_endline("STREAM FINISHED")
      };
      Printf.sprintf(
        "RESULT: %s",
        Knot.Util.print_optional(Debug.print_tkn, result),
      )
      |> print_endline;
      Some((tkn, stream));

    /* reached the EOF */
    | (None, LazyStream.Nil) => None

    /* unmatched character found */
    | (None, LazyStream.Cons((ch, cursor), _)) =>
      throw_syntax(InvalidCharacter(ch, cursor))
    };
  } else {
    switch (stream) {
    /* has remaining characters */
    | LazyStream.Cons((ch, _), next_stream) =>
      Buffer.add_utf_8_uchar(buf, ch);

      let (next_result, next_ms) =
        Matcher.execute_each(result, buf, stream, matchers);

      _find_token(
        ~result=next_result,
        ~buf,
        next_ms,
        List.length(next_ms) == 0 ? stream : Lazy.force(next_stream),
      );
    /* reached the EOF */
    | LazyStream.Nil =>
      /* throw errors for unclosed matchers */
      Matcher.find_unclosed(matchers) |*> throw_syntax;

      result |?> (r => Some((r, LazyStream.Nil)));
    };
  };

let next_token = _find_token(_root_matchers);
