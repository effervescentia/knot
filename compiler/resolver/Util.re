open Core;

/* TODO: give these unique IDs within the scope */
let generate_any_type = () => Any_t(0);

let typeof =
  fun
  | (_, {contents: Resolved(t)}) => Some(t)
  | _ => None;

let is_resolved = prom =>
  typeof(prom)
  |> (
    fun
    | Some(_) => true
    | None => false
  );
