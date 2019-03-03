open Core;

let rec generate = gen_expression =>
  fun
  | Variable(name) => name
  | DotAccess(lhs, rhs) =>
    Printf.sprintf(
      "%s%s",
      abandon_ctx(lhs) |> generate(gen_expression),
      rhs |> Property.gen_access,
    )
  | Execution(target, args) =>
    Printf.sprintf(
      "%s(%s)",
      abandon_ctx(target) |> generate(gen_expression),
      gen_list(abandon_ctx % gen_expression, args),
    );
