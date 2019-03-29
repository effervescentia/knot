open Core;

let gen_member = name =>
  fun
  | `Mutator(params, exprs)
  | `Getter(params, exprs) =>
    Printf.sprintf(
      "%s:function%s",
      Property.gen_key(name),
      Function.gen_body(params, exprs),
    )
  | `Property(type_def, default_val) =>
    Printf.sprintf(
      "%s:%s",
      Property.gen_key(name),
      switch (default_val) {
      | Some((expr, _)) => Expression.generate(expr)
      | None => "undefined"
      },
    );
