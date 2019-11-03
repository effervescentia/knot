open Core;

let rec test_match = (stream, matcher) =>
  switch (stream) {
  | LazyStream.Cons((c, _), next_stream) =>
    switch (matcher) {
    | Char(ch) when Uchar.equal(Uchar.of_char(ch), c) =>
      Some(Lazy.force(next_stream))
    | Alpha when Util.is_uchar_alpha(c) => Some(Lazy.force(next_stream))
    | Numeric when Util.is_uchar_numeric(c) => Some(Lazy.force(next_stream))
    | AlphaNumeric when Util.is_uchar_alphanumeric(c) =>
      Some(Lazy.force(next_stream))
    | Any => Some(Lazy.force(next_stream))
    | Either(ms) => _first_match(test_match(stream), ms)
    | Except(ms) =>
      _has_matches(test_match(stream), ms)
        ? None : Some(Lazy.force(next_stream))
    | Token(s) =>
      let rec next = (s, input) =>
        switch (input) {
        | LazyStream.Cons((c, _), next_input)
            when Uchar.equal(c, Uchar.of_char(s.[0])) =>
          if (String.length(s) == 1) {
            Some(Lazy.force(next_input));
          } else {
            next(
              String.sub(s, 1, String.length(s) - 1),
              Lazy.force(next_input),
            );
          }
        | _ => None
        };

      next(s, stream);
    | _ => None
    }
  | LazyStream.Nil =>
    switch (matcher) {
    | Except(_)
    | Any => Some(stream)
    | _ => None
    }
  }
and _first_match = test =>
  fun
  | [m, ...ms] => test(m) |=> (() => _first_match(test, ms))
  | [] => None
and _has_matches = test => List.exists(m => has_opt(test(m)));

// let rec test_match = (stream, matcher) => {
//   // switch (stream) {
//   // | LazyStream.Cons((c, _), _) =>
//   //   print_endline(Printf.sprintf("CHAR: %s", Knot.Util.print_uchar(c)))
//   // | _ => ()
//   // };
//   // Debug.print_matcher(matcher)
//   // |> Printf.sprintf("MATCHER: %s")
//   // |> print_endline;
//   switch (stream, matcher) {
//   /* match character */
//   | (LazyStream.Cons((c, _), next_stream), Char(ch))
//       when Uchar.equal(Uchar.of_char(ch), c) =>
//     Some(Lazy.force(next_stream))

//   /* match alpha */
//   | (LazyStream.Cons((c, _), next_stream), Alpha)
//       when Util.is_uchar_alpha(c) =>
//     Some(Lazy.force(next_stream))

//   /* match alphanumeric */
//   | (LazyStream.Cons((c, _), next_stream), AlphaNumeric)
//       when Util.is_uchar_alphanumeric(c) =>
//     Some(Lazy.force(next_stream))

//   /* match token */
//   | (LazyStream.Cons(_), Token(pattern)) =>
//     let rec next = (s, input) =>
//       switch (input) {
//       | LazyStream.Cons((c, _), next_input)
//           when Uchar.equal(c, Uchar.of_char(s.[0])) =>
//         if (String.length(s) == 1) {
//           Some(Lazy.force(next_input));
//         } else {
//           next(
//             String.sub(s, 1, String.length(s) - 1),
//             Lazy.force(next_input),
//           );
//         }
//       | _ => None
//       };

//     next(pattern, stream);

//   /* match any */
//   | (LazyStream.Cons(_, next_stream), Any) => Some(Lazy.force(next_stream))

//   /* match whitelist */
//   | (LazyStream.Cons(_), Either(ms)) => _first_match(test_match(stream), ms)

//   /* match blacklist */
//   | (LazyStream.Cons(_, next_stream), Except(ms)) =>
//     _has_matches(test_match(stream), ms)
//       ? None : Some(Lazy.force(next_stream))

//   /* lenient matchers */
//   | (LazyStream.Nil, Any | Except(_)) => Some(LazyStream.Nil)

//   | _ => None
//   };
// }
// and _first_match = test =>
//   fun
//   | [m, ...ms] => test(m) |=> (() => _first_match(test, ms))
//   | [] => None
// and _has_matches = test => List.exists(test % has_opt);

let rec execute = (result, buf, stream, matcher) => {
  // (
  //   switch (stream) {
  //   | LazyStream.Cons((c, (row, col)), _) =>
  //     Printf.sprintf(
  //       "char(%s) at [%d:%d]",
  //       Knot.Util.print_uchar(c),
  //       row,
  //       col,
  //     )
  //   | _ => ""
  //   }
  // )
  // |> print_endline;
  // Knot.Util.print_optional(
  //   Debug.print_tkn % Printf.sprintf("CURR RESULT: %s\n"),
  //   result,
  // )
  // |> print_string;
  // Debug.print_lex_matcher(matcher)
  // |> Printf.sprintf("TEST: %s\n")
  // |> print_string;
  (
    fun
    /* simple match of current character */
    | Matcher(m, t)
    | TerminalMatcher(_, m, t) => {
        switch (test_match(stream, m)) {
        | Some(str) =>
          // print_endline(
          //   Printf.sprintf("MATCHED: %s", Debug.print_matcher(m)),
          // );
          t(Buffer.contents(buf), str)
        | None =>
          // print_endline(
          //   Printf.sprintf("DID NOT MATCH: %s", Debug.print_matcher(m)),
          // );
          (result, [], stream)
        };
      }

    /* match current and next character */
    | LookaheadMatcher(m, nm, t) =>
      switch (test_match(stream, m)) {
      | Some(next_stream) =>
        // print_endline(
        //   Printf.sprintf("MATCHED LOOKAHEAD: %s", Debug.print_matcher(m)),
        // );
        execute(result, buf, next_stream, Matcher(nm, t))
      | None => (result, [], stream)
      }
  )(
    matcher,
  );
};

let rec execute_each = (result, buf, stream) =>
  fun
  /* no matchers */
  | [] => {
      (
        // Printf.sprintf(
        //   " RESULT: %s",
        //   Knot.Util.print_optional(Debug.print_tkn, result),
        // )
        // |> print_endline;
        result,
        [],
      );
    }

  /* execute each matcher */
  | ms => {
      // Printf.sprintf(
      //   "POTENTIAL RESULT: %s",
      //   Knot.Util.print_optional(Debug.print_tkn, result),
      // )
      // |> print_endline;
      // print_endline("MATCHERS");
      // Knot.Util.print_sequential(~separator="\n", Debug.print_lex_matcher, ms)
      // |> print_endline;
      let (n_res, n_ms) =
        List.fold_left(
          ((res, acc), m) => {
            let (next_res, next_matchers, next_stream) =
              execute(res, buf, stream, m);

            // Printf.sprintf(
            //   "  INNER RESULT: %s",
            //   Knot.Util.print_optional(Debug.print_tkn, next_res),
            // )
            // |> print_endline;
            (
              next_res |=> (() => res),
              List.length(next_matchers) === 0 ? acc : next_matchers @ acc,
            );
          },
          (result, []),
          ms,
        );

      // Printf.sprintf(
      //   "RESULT: %s",
      //   Knot.Util.print_optional(Debug.print_tkn, n_res),
      // )
      // |> print_endline;

      (n_res, n_ms);
    };

let rec find_unclosed =
  fun
  | [] => None
  | [TerminalMatcher(e, _, _)] => Some(e)
  | ms =>
    List.fold_left(
      (acc, m) => acc |=> (() => find_unclosed([m])),
      None,
      ms,
    );
