open Core;

let rec generate = gen_expression =>
  fun
  | A_Variable(name) => name
  | A_DotAccess(lhs, rhs) =>
    Printf.sprintf(
      "%s%s",
      unwrap(lhs) |> generate(gen_expression),
      unwrap(rhs) |> generate(gen_expression) |> Property.gen_access,
    )
  | A_Execution(target, args) =>
    Printf.sprintf(
      "%s(%s)",
      unwrap(target) |> generate(gen_expression),
      gen_list(unwrap % gen_expression, args),
    );