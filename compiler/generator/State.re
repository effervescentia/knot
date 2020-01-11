open Core;

let gen_member = name =>
  fun
  | `Mutator(params, exprs) =>
    Printf.sprintf(
      "%s(\"%s\", (function %s))",
      mutator_factory,
      name,
      Function.gen_body(Expression.generate, params, exprs),
    )
  | `Getter(params, exprs) =>
    Printf.sprintf(
      "function%s",
      Function.gen_body(Expression.generate, params, exprs),
    )
  | `Property(type_def, default_val) =>
    switch (default_val) {
    | Some((expr, _)) =>
      Expression.generate(expr)
      |> Printf.sprintf("%s(\"%s\", %s)", property_factory, name)
    | None => "undefined"
    };
