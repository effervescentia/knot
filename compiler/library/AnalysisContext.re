type eventual_ctx('a) =
  | Unanalyzed
  | Pending(list('a => unit))
  | Resolved('a);

type ctxl_promise('a) = ('a, ref(eventual_ctx(MemberType.member_type)));

let no_ctx = x => (x, ref(Unanalyzed));
