open Core;

type context_action =
  | PushContext(context)
  | PopContext;

let mutate_context = context =>
  fun
  | PushContext(ctx) => context := [ctx, ...context^]
  | PopContext => context := List.tl(context^);

type file_streamer('a) =
  LazyStream.t((char, context_action => LazyStream.t((char, 'a)))) => unit;

let of_file_stream2 = file_stream => {
  let stream = ref(file_stream);
  let context = ref([]);

  /* ((tkn, next_stream), Add(context) | Remove(context)) => (tkn, next_stream) */

  let rec next = (curr_stream, curr_context, handle_action) =>
    switch (Lexer.next_token(curr_stream, curr_context)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;
      Some((tkn, handle_action));
    | None => None
    };

  LazyStream.of_function(() =>
    next(
      stream^,
      context^,
      action => {
        mutate_context(context, action);
        next(stream^, context^);
      },
    )
  );
};

/* let of_file_stream = file_stream => {
     let stream = ref(file_stream);

     let rec next = curr_stream =>
       switch (Lexer.next_token(curr_stream)) {
       | Some((tkn, next_stream)) =>
         stream := next_stream;
         Some(tkn);
       | None => None
       };

     LazyStream.of_function(() => next(stream^));
   }; */

let without_comments2 = token_stream => {
  let cursor = ref(token_stream);

  let rec next = () =>
    switch (cursor^) {
    | LazyStream.Cons((x, _) as res, input) =>
      cursor := Lazy.force(input);
      switch (x) {
      | LineComment(_)
      | BlockComment(_) => next()
      | _ => Some(res)
      };
    | LazyStream.Nil => None
    };

  LazyStream.of_function(next);
};

/* let without_comments = token_stream => {
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
   }; */
