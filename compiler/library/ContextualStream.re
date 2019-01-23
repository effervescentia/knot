open Globals;

type context_action('a) =
  | PushContext('a)
  | PopContext;

let of_function = (initial_context, f) => {
  let context = ref([initial_context]);

  let mutate_context = ctxs =>
    fun
    | PushContext(ctx) => [ctx, ...ctxs]
    | PopContext => List.tl(ctxs);

  let rec next = (f, ctxs) =>
    switch (f()) {
    | Some(x) =>
      LazyStream.Cons(
        (
          x,
          List.hd(context^),
          (action => lazy (mutate_context(ctxs, action) |> next(f))),
        ),
        lazy (next(f, ctxs)),
      )
    | None => Nil
    };
  next(f, [initial_context]);
};
