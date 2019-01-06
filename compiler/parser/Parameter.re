open Core;

module M = Matchers;

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

let params = M.parentheses(comma_separated(param));