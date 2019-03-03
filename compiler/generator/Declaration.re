open Core;

let gen_export = name =>
  Printf.sprintf("%s%s=%s;", export_map, Property.gen_access(name), name);

let generate = printer =>
  abandon_ctx
  % (
    fun
    | ConstDecl(name, expr) =>
      Printf.sprintf(
        "var %s=%s;%s",
        name,
        abandon_ctx(expr) |> Expression.generate,
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
        "function %s(){%s%s}%s",
        name,
        List.map(abandon_ctx, params) |> Function.gen_params,
        gen_list(abandon_ctx % State.gen_prop, props)
        |> Printf.sprintf("return {%s};"),
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
        "function %s(){%s%s}%s",
        name,
        List.map(abandon_ctx, params) |> Function.gen_params,
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
  )
  % printer;
