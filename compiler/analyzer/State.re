open Core;

let analyze_prop = scope =>
  abandon_ctx
  % (
    fun
    | Property(prop) => {
        let (name, type_def, default_val) = abandon_ctx(prop);

        Property.analyze_type_def(type_def) |> ignore;
        switch (default_val) {
        | Some(expr) => Expression.analyze(scope, expr)
        | None => ()
        };
      }
    | Mutator(name, params, exprs) => {
        List.iter(Property.analyze(Expression.analyze, scope), params);
        List.iter(Expression.analyze(scope), exprs);
      }
    | Getter(name, params, exprs) => {
        List.iter(Property.analyze(Expression.analyze, scope), params);
        List.iter(Expression.analyze(scope), exprs);
      }
  );
