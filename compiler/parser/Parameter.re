open Core;

let assignment = M.assign >> Expression.expr ==> (e => Some(e));
let param =
  M.identifier
  >>= (
    name =>
      M.type_def
      >>= (
        type_def =>
          assignment
          |= None
          ==> (default_val => (name, type_def, default_val))
      )
  );

let params = M.comma_separated(param) |> M.parentheses;