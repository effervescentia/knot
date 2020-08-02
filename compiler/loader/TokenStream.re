open Core;

let filter_comments =
  fun
  | LineComment(_)
  | BlockComment(_) => false
  | _ => true;

let of_file_stream = (~filter=Some(filter_comments), tokenize, file_stream) => {
  let stream = ref(file_stream);

  let rec next = () =>
    switch (tokenize(stream^)) {
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
