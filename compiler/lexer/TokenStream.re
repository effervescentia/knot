open Knot;

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