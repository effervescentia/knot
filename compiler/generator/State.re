open Core;

let gen_prop =
  fun
  | Mutator(name, params, exprs)
  | Getter(name, params, exprs) =>
    Printf.sprintf(
      "%s:function%s",
      Property.gen_key(name),
      Function.gen_body(params, exprs),
    )
  | Property(prop) => {
      let (name, type_def, default_val) = abandon_ctx(prop);

      Printf.sprintf(
        "%s:%s",
        Property.gen_key(name),
        switch (default_val) {
        | Some(expr) => abandon_ctx(expr) |> Expression.generate
        | None => "undefined"
        },
      );
    };
