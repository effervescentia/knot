open Globals;

let generate = (printer, core, name) =>
  fst
  % (
    fun
    | ConstDecl((expr, _)) =>
      Printf.sprintf(
        "var %s=%s;%s",
        name,
        Expression.generate(expr),
        core.to_export_statement(name, None),
      )

    | FunctionDecl(params, exprs) =>
      Printf.sprintf(
        "function %s%s%s",
        name,
        Function.gen_body(Expression.generate, params, exprs),
        core.to_export_statement(name, None),
      )

    | StateDecl(params, props) =>
      Printf.sprintf(
        "function %s(%s){%s%s%s}%s",
        name,
        state_factory,
        List.map(fst, params) |> Function.gen_params(Expression.generate),
        gen_terminated(
          ((name, (prop, _))) =>
            State.gen_member(name, prop)
            |> Printf.sprintf("var $%s=%s", name),
          props,
        ),
        gen_list(
          ((name, (prop, _))) =>
            Printf.sprintf("%s:$%s", Property.gen_key(name), name),
          props,
        )
        |> Printf.sprintf(
             "return {get:function(){return %s({%s});}};",
             factory_constructor,
           ),
        core.to_export_statement(name, None),
      )

    | ViewDecl(_, mixins, props, exprs) =>
      Printf.sprintf(
        "%s%s",
        View.generate(name, mixins, props, exprs),
        core.to_export_statement(name, None),
      )

    | StyleDecl(params, rule_sets) =>
      Printf.sprintf(
        "function %s(){%s%s}%s",
        name,
        List.map(fst, params) |> Function.gen_params(Expression.generate),
        gen_list(Style.gen_rule_set, rule_sets)
        |> Printf.sprintf("return {%s};"),
        core.to_export_statement(name, None),
      )
  )
  % printer;
