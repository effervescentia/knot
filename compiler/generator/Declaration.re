open Core;

let gen_export = name =>
  Printf.sprintf("%s%s=%s;", export_map, Property.gen_access(name), name);

let generate = printer =>
  unwrap
  % (
    fun
    | A_ConstDecl(name, expr) =>
      Printf.sprintf(
        "var %s=%s;%s",
        name,
        unwrap(expr) |> Expression.generate,
        gen_export(name),
      )
    | A_FunctionDecl(name, params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(params, exprs),
        gen_export(name),
      )
    | A_StateDecl(name, params, props) =>
      Printf.sprintf(
        "function %s(){%s%s}%s",
        name,
        List.map(unwrap, params) |> Function.gen_params,
        gen_list(unwrap % State.gen_prop, props)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
    | A_ViewDecl(name, _, _, params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(params, exprs),
        gen_export(name),
      )
    | A_StyleDecl(name, params, rule_sets) =>
      Printf.sprintf(
        "function %s(){%s%s}%s",
        name,
        List.map(unwrap, params) |> Function.gen_params,
        gen_list(unwrap % Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
  )
  % printer;