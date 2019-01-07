open Core;

let of_file_stream = file_stream => {
  let stream = ref(file_stream);

  let next = () =>
    switch (Lexer.next_token(stream^)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;
      Some(tkn);
    | None => None
    };

  LazyStream.of_function(next);
};

let without_comments = token_stream => {
  let cursor = ref(token_stream);

  let rec next = () =>
    switch (cursor^) {
    | LazyStream.Cons(x, input) =>
      cursor := Lazy.force(input);
      switch (x) {
      | LineComment(_)
      | BlockComment(_) => next()
      | _ => Some(x)
      };
    | LazyStream.Nil => None
    };

  LazyStream.of_function(next);
};