open Core;

let analyze_prop = scope =>
  fst
  % (
    fun
    | Property(prop) => {
        let (name, type_def, default_val) = fst(prop);

        Property.analyze_type_def(scope, type_def);
        switch (default_val) {
        | Some(expr) => Expression.analyze(scope, expr)
        | None => ()
        };
      }
    | Mutator(name, params, exprs) => {
        List.iter(Property.analyze(Expression.analyze, scope), params);
        List.iter(Function.analyze_scoped_expr(scope), exprs);
      }
    | Getter(name, params, exprs) => {
        List.iter(Property.analyze(Expression.analyze, scope), params);
        List.iter(Function.analyze_scoped_expr(scope), exprs);
      }
  );
