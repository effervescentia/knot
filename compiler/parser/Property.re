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
  (M.comma_separated(prop) |> M.parentheses ==> List.map(no_ctx))(input);
