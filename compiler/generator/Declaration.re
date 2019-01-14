open Core;

let gen_export = name =>
  Printf.sprintf("%s['%s']=%s;", export_map, name, name);

let generate = printer =>
  (
    fun
    | ConstDecl(name, expr) =>
      Printf.sprintf(
        "var %s=%s;%s",
        name,
        Expression.generate(expr),
        gen_export(name),
      )
    | FunctionDecl(name, params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(params, exprs),
        gen_export(name),
      )
    | StateDecl(name, params, props) =>
      Printf.sprintf(
        "function %s(%s){%s%s}%s",
        name,
        Function.args_map,
        gen_terminated(Function.gen_param, params),
        gen_list(State.gen_prop, props) |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
    | ViewDecl(name, _, _, params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(params, exprs),
        gen_export(name),
      )
    | StyleDecl(name, params, rule_sets) =>
      Printf.sprintf(
        "function %s(%s){%s%s}%s",
        name,
        Function.args_map,
        gen_terminated(Function.gen_param, params),
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
  )
  % printer;