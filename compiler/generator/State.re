open Core;

let gen_member =
  fun
  | `Mutator(params, exprs) =>
    Printf.sprintf(
      "function(){function $$_mutator%s$$_mutator.apply(null, arguments);$$_update();}",
      Function.gen_body(params, exprs),
    )
  | `Getter(params, exprs) =>
    Printf.sprintf("function%s", Function.gen_body(params, exprs))
  | `Property(type_def, default_val) =>
    switch (default_val) {
    | Some((expr, _)) => Expression.generate(expr)
    | None => "undefined"
    };
