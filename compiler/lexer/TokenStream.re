open Core;

module ContextualStream = Knot.ContextualStream;

let of_file_stream = (initial_context, filter, file_stream) => {
  let stream = ref(file_stream);

  let rec next = () =>
    switch (Lexer.next_token(stream^)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;

      filter(tkn) ? Some(tkn) : next();
    | None => None
    };

  ContextualStream.of_function(initial_context, next);
};
