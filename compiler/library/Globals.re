module LazyStream = Opal.LazyStream;

let (%) = (f, g, x) => f(x) |> g;

type eventual_ctx('a, 'b) =
  | Pending('a)
  | Resolved('a, 'b);

type ctxl_promise('a) = ref(eventual_ctx('a, unit));

let opt_transform = transform =>
  fun
  | Some(x) => Some(transform(x))
  | None => None;

let await_ctx = x => ref(Pending(x));

let abandon_ctx = x =>
  switch (x^) {
  | Pending(res) => res
  | Resolved(res, _) => res
  };

let opt_abandon_ctx =
  fun
  | Some(x) => Some(abandon_ctx(x))
  | None => None;

let reserved = [
  "import",
  "from",
  "const",
  "let",
  "func",
  "view",
  "state",
  "style",
  "else",
  "if",
  "mut",
  "get",
  "main",
  "true",
  "false",
];
