open Core;

let gen_export = name => Printf.sprintf("export {%s};", name);

let generate = (printer, name) =>
  fst
  % (
    fun
    | ConstDecl((expr, _)) =>
      Printf.sprintf(
        "var %s=%s;%s",
        name,
        Expression.generate(expr),
        gen_export(name),
      )

    | FunctionDecl(params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(params, exprs),
        gen_export(name),
      )

    | StateDecl(params, props) =>
      Printf.sprintf(
        "function %s(){%s%s}%s",
        name,
        List.map(fst, params) |> Function.gen_params,
        gen_list(
          ((name, (prop, _))) => State.gen_member(name, prop),
          props,
        )
        |> Printf.sprintf(
             "var $$_state={%s};return {get:function(){return $$_state;}};",
           ),
        gen_export(name),
      )

    | ViewDecl(_, _, props, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        View.generate(props, exprs),
        gen_export(name),
      )

    | StyleDecl(params, rule_sets) =>
      Printf.sprintf(
        "function %s(){%s%s}%s",
        name,
        List.map(fst, params) |> Function.gen_params,
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        gen_export(name),
      )
  )
  % printer;
