include Knot.Globals;
include Knot.Token;

type char_pointer = Knot.UnicodeFileStream.char_pointer;

type match_pattern =
  | Alpha
  | Numeric
  | AlphaNumeric
  | Char(char)
  | Token(string)
  /* pattern combinators */
  | Any
  | Either(list(match_pattern))
  | Except(list(match_pattern));

type matcher =
  | Matcher(
      match_pattern,
      (string, LazyStream.t(char_pointer)) => match_result,
    )
  | LookaheadMatcher(
      match_pattern,
      match_pattern,
      (string, LazyStream.t(char_pointer)) => match_result,
    )
  | TerminalMatcher(
      Knot.Exception.syntax_error,
      match_pattern,
      (string, LazyStream.t(char_pointer)) => match_result,
    )
and match_result = (
  option(Knot.Core.token),
  list(matcher),
  LazyStream.t(char_pointer),
);

let result = (res, stream) => (Some(res), [], stream);

let matcher_list = (ms, stream) => (None, ms, stream);

let single_matcher = m => matcher_list([m]);

let empty_matchers = str => matcher_list([], str);

let rec match_contiguous = (s, t, f) => {
  // Printf.sprintf("CONTIGUOUS: %s", s) |> print_endline;

  let next = _ =>
    if (String.length(s) == 1) {
      result(t);
    } else {
      String.sub(s, 1, String.length(s) - 1) |> f;
    };

  Matcher(Char(s.[0]), next);
};

/* lexer matcher for a contiguous set of characters */
let rec (===>) = (s, t) =>
  match_contiguous(s, t, ss => single_matcher(ss ===> t));
