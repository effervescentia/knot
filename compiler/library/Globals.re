module LazyStream = Opal.LazyStream;

exception NotImplemented;

let (%) = (f, g, x) => f(x) |> g;

type member_type =
  | Number_t
  | String_t
  | Boolean_t
  | Array_t(member_type)
  | Object_t(Hashtbl.t(string, member_type))
  | Function_t(list(member_type), member_type)
  | JSX_t
  | View_t
  | State_t
  | Style_t
  | Module_t(
      list(string),
      Hashtbl.t(string, member_type),
      option(member_type),
    )
  | Any_t(int)
  | Nil_t;

type eventual_ctx('a, 'b) =
  | Unanalyzed('a)
  | Pending('a, list(('a, 'b) => unit))
  | Resolved('a, 'b);

type ctxl_promise('a) = ref(eventual_ctx('a, member_type));

let no_ctx = x => ref(Unanalyzed(x));

let await_ctx = x => ref(Pending(x, []));

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
