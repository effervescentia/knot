open Core;

let analyze_prop = (scope, name, promise) =>
  switch (fst(promise)) {
  | `Property(type_def, default_val) as res =>
    Property.analyze_type_def(scope, type_def);
    Property.analyze_default_value(Expression.analyze(scope), default_val);

    Resolver.of_state_property(name, (res, snd(promise))) |> scope.resolve;

  | `Mutator(params, exprs) =>
    let nested_scope = scope.nest();

    List.iter(Property.analyze(Expression.analyze, nested_scope), params);
    List.iter(Function.analyze_scoped_expr(nested_scope), exprs);

  | `Getter(params, exprs) =>
    let nested_scope = scope.nest();

    List.iter(Property.analyze(Expression.analyze, nested_scope), params);
    List.iter(Function.analyze_scoped_expr(nested_scope), exprs);
  };
