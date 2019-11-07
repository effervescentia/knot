open Core;

let _root_matchers =
  Character.matchers
  @ Keyword.matchers
  @ Pattern.matchers
  @ Text.matchers
  @ Identifier.matchers
  @ Number.matchers
  @ Boolean.matchers
  @ Comment.matchers;

let _create_matchers = stream =>
  _root_matchers
  |> List.map(matcher =>
       Matcher.{matcher, initial: stream, current: stream, length: 1}
     );

let rec find_token = (~result=None, matchers) => {
  switch (result, matchers) {
  /* no result */
  | (None, []) => None
  /* some result */
  | (Some(_) as res, []) => res
  /* remaining matchers to resolve */
  | (_, [matcher, ...rest_matchers]) =>
    let (next_result, next_matchers) = Matcher.resolve(result, matcher);

    find_token(~result=next_result, rest_matchers @ next_matchers);
  };
};

let next_token = stream =>
  switch (find_token(_create_matchers(stream)), stream) {
  | (None, LazyStream.Nil) => None
  | (None, LazyStream.Cons((c, cursor), _)) =>
    throw_syntax(InvalidCharacter(c, cursor))
  | (Some(_) as res, _) => res
  };
