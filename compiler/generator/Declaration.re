open Core;

let gen_export = name =>
  Printf.sprintf("%s%s=%s;", export_map, Property.gen_access(name), name);

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
        "function %s(){%s%s}%s",
        name,
        Function.gen_params(params),
        gen_list(State.gen_prop, props) |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
    | ViewDecl(name, super, mixins, params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(
          ~preamble=
            switch (mixins) {
            | [] => ""
            | [x, ...xs] =>
              Printf.sprintf("var %s=%s.style(%s());", class_map, util_map, x)
            },
          params,
          exprs,
        ),
        gen_export(name),
      )
    | StyleDecl(name, params, rule_sets) =>
      Printf.sprintf(
        "var %s=%s.memo(function(){%s%s});%s",
        name,
        util_map,
        Function.gen_params(params),
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
  )
  % printer;