open Core;

type type_t =
  | Synthetic_t(member_type)
  | Resolved_t(member_type)
  | Untyped_t;

let typeof =
  fun
  | (_, {contents: {contents: Resolved(t) | Synthetic(t, _)}}) => Some(t)
  | _ => None;

let typeof_2 = ((_, promise)) =>
  switch (promise^) {
  | Resolved(t) => Resolved_t(t)
  | Synthetic(t, []) => Synthetic_t(t)
  | _ => Untyped_t
  };

let get_type = x =>
  switch (typeof(x)) {
  | Some(t) => t
  | None => raise(TypeNotSet)
  };

let is_resolved = prom =>
  typeof(prom)
  |> (
    fun
    | Some(_) => true
    | None => false
  );
