open Core;

let assignment = M.assign >> Expression.expr ==> (e => Some(e));

let prop =
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