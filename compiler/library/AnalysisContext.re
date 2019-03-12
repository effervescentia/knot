type eventual_ctx('a, 'b) =
  | Unanalyzed('a)
  | Pending('a, list(('a, 'b) => unit))
  | Resolved('a, 'b);

type ctxl_promise('a) = ref(eventual_ctx('a, MemberType.member_type));

let no_ctx = x => ref(Unanalyzed(x));

let abandon_ctx = x =>
  switch (x^) {
  | Unanalyzed(res)
  | Pending(res, _)
  | Resolved(res, _) => res
  };

let opt_abandon_ctx =
  fun
  | Some(x) => Some(abandon_ctx(x))
  | None => None;

let opt_transform = transform =>
  fun
  | Some(x) => Some(transform(x))
  | None => None;
