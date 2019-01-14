open Core;

let generate = printer =>
  (
    fun
    | ConstDecl(name, expr) =>
      Expression.generate(expr) |> Printf.sprintf("var %s=%s;", name)
    | FunctionDecl(name, params, exprs) =>
      Printf.sprintf("function %s%s", name, Function.gen_body(params, exprs))
    | StateDecl(name, params, props) =>
      Printf.sprintf(
        "function %s(%s){%s%s}",
        name,
        Function.args_map,
        gen_terminated(Function.gen_param, params),
        gen_list(State.gen_prop, props) |> Printf.sprintf("return {%s};"),
      )
    | ViewDecl(name, _, _, params, exprs) =>
      Printf.sprintf("function %s%s", name, Function.gen_body(params, exprs))
    | StyleDecl(name, params, rule_sets) =>
      Printf.sprintf(
        "function %s(%s){%s%s}",
        name,
        Function.args_map,
        gen_terminated(Function.gen_param, params),
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
      )
  )
  % printer;