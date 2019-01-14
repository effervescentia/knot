open Core;

let gen_prop =
  fun
  | Mutator(name, params, exprs)
  | Getter(name, params, exprs) =>
    Printf.sprintf(
      "['%s']:function%s",
      name,
      Function.gen_body(params, exprs),
    )
  | Property(name, _, default_val) =>
    Printf.sprintf(
      "['%s']:%s",
      name,
      switch (default_val) {
      | Some(expr) => Expression.generate(expr)
      | None => "undefined"
      },
    );