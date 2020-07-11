open Core;

let of_file_stream = (~filter=?, file_stream) => {
  let stream = ref(file_stream);

  let rec next = () =>
    switch (Lexer.next_token(stream^)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;

      switch (filter) {
      | Some(f) when !f(tkn) => next()
      | _ => Some(tkn)
      };
    | None => None
    };

  LazyStream.of_function(next);
};

let filter_comments =
  fun
  | {token: LineComment(_) | BlockComment(_)} => false
  | _ => true;
