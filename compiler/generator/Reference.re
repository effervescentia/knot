open Globals;

let rec generate = gen_expression =>
  fun
  | Variable(name) => name
  | SidecarVariable(name) => Printf.sprintf("$%s", name)
  | DotAccess((lhs, _), rhs) =>
    Printf.sprintf(
      "%s%s",
      generate(gen_expression, lhs),
      rhs |> Property.gen_access,
    )
  | Execution((target, _), args) =>
    Printf.sprintf(
      "%s(%s)",
      generate(gen_expression, target),
      gen_list(fst % gen_expression, args),
    );
