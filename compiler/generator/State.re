open Core;

let gen_prop =
  fun
  | A_Mutator(name, params, exprs)
  | A_Getter(name, params, exprs) =>
    Printf.sprintf(
      "%s:function%s",
      Property.gen_key(name),
      Function.gen_body(params, exprs),
    )
  | A_Property((name, _, default_val)) =>
    Printf.sprintf(
      "%s:%s",
      Property.gen_key(name),
      switch (default_val) {
      | Some(expr) => unwrap(expr) |> Expression.generate
      | None => "undefined"
      },
    );