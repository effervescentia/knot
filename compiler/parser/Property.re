open Core;

let assignment = input =>
  (M.assign >> Expression.expr ==> no_ctx ==> (e => Some(e)))(input);

let prop = input =>
  (
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
    )
  )(
    input,
  );

let list = input =>
  (prop ==> no_ctx |> M.comma_separated |> M.parentheses)(input);
