open Core;

exception InvalidContextManipulation;

let of_file_stream = (~filter=?, file_stream) => {
  let stream = ref(file_stream);
  let context_stack = ref([Normal]);

  let update_ctx = action => {
    Debug.print_action(action)
    |> Printf.sprintf("context action: %s")
    |> print_endline;
    switch (action, context_stack^) {
    | (NoOp, _) => ()
    | (PushContext(ctx), ctxs) => context_stack := [ctx, ...ctxs]
    | (SwapContext(_), [])
    | (PopContext, []) => raise(InvalidContextManipulation)
    | (SwapContext(ctx), ctxs) => context_stack := [ctx, ...List.tl(ctxs)]
    | (PopContext, ctxs) => context_stack := List.tl(ctxs)
    };
  };

  let rec next = () =>
    switch (Lexer.next_token(List.hd(context_stack^), stream^)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;

      switch (filter) {
      | Some(f) when !f(tkn) => next()
      | _ => Some((tkn, update_ctx))
      };
    | None => None
    };

  LazyStream.of_function(next);
};

let filter_comments =
  fun
  | LineComment(_)
  | BlockComment(_) => false
  | _ => true;
