open Core;

module M = Matchers;

let definition = M.colon >> M.identifier ==> (t => Some(t));
let assignment = M.assign >> Expression.expr ==> (e => Some(e));

let prop =
  M.terminated(
    M.identifier
    >>= (
      k =>
        definition
        |= None
        >>= (
          type_def =>
            assignment
            |= None
            ==> (default_val => (k, (type_def, default_val)))
        )
    ),
  );