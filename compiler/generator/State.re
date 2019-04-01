open Core;

let gen_member =
  fun
  | `Mutator(params, exprs) =>
    Printf.sprintf(
      "function(){(function %s).apply(null, arguments);%s();}",
      Function.gen_body(Expression.generate, params, exprs),
      update_handler,
    )
  | `Getter(params, exprs) =>
    Printf.sprintf(
      "function%s",
      Function.gen_body(Expression.generate, params, exprs),
    )
  | `Property(type_def, default_val) =>
    switch (default_val) {
    | Some((expr, _)) => Expression.generate(expr)
    | None => "undefined"
    };
