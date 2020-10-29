open Globals;

let assignment = (parse_expr, input) =>
  (M.assign >> parse_expr ==> no_ctx ==> (e => Some(e)))(input);

let prop = (parse_expr, input) =>
  (
    M.identifier
    >>= (
      name =>
        M.type_def
        >>= (
          type_def =>
            assignment(parse_expr)
            |= None
            ==> (default_val => (name, type_def, default_val))
        )
    )
  )(
    input,
  );

let list = (parse_expr, input) =>
  (
    prop(parse_expr)
    |> M.comma_separated
    |> M.parentheses
    ==> List.map(no_ctx)
  )(
    input,
  );
