type eventual_ctx('a) =
  | Unanalyzed
  | Synthetic('a, list(unit)) /* for type casting */
  | Resolved('a);

type ctxl_promise('a) = (
  'a,
  ref(ref(eventual_ctx(MemberType.member_type))),
);

let no_ctx = x => (x, ref(ref(Unanalyzed)));
