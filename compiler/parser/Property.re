open Core;

let assignment = M.assign >> Expression.expr ==> (e => Some(e));

let prop =
  M.identifier
  >>= (
    k =>
      M.type_def
      >>= (
        type_def =>
          assignment
          |= None
          ==> (default_val => (k, (type_def, default_val)))
      )
  )
  |> M.terminated;