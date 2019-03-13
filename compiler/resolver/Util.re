open Core;

let typeof =
  fun
  | (_, {contents: Resolved(t) | Synthetic(t, _)}) => Some(t)
  | _ => None;

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
