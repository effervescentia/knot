open Core;

exception InvalidContextManipulation;

let of_file_stream = file_stream => {
  let stream = ref(file_stream);
  let context_stack = ref([Normal]);

  let update_ctx = action =>
    switch (action, context_stack^) {
    | (NoOp, _) => ()
    | (PushContext(ctx), ctxs) => context_stack := [ctx, ...ctxs]
    | (SwitchContext(_), [])
    | (PopContext, []) => raise(InvalidContextManipulation)
    | (SwitchContext(ctx), ctxs) => context_stack := [ctx, ...List.tl(ctxs)]
    | (PopContext, ctxs) => context_stack := List.tl(ctxs)
    };

  let next = () =>
    switch (Lexer.next_token(List.hd(context_stack^), stream^)) {
    | Some((tkn, next_stream)) =>
      stream := next_stream;
      Some((tkn, update_ctx));
    | None => None
    };

  LazyStream.of_function(next);
};
