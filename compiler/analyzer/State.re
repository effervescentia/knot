open Globals;

let analyze_prop = (scope, name, promise) =>
  switch (fst(promise)) {
  | `Property(type_def, default_val) as res =>
    Property.analyze_type_def(scope, type_def);
    Property.analyze_default_value(Expression.analyze(scope), default_val);

    Resolver.of_state_property(name, (res, snd(promise))) |> scope.resolve;

  | `Mutator(params, exprs) as res =>
    let nested_scope =
      scope.nest(~label=Printf.sprintf("mut(%s)", name), ());

    Function.analyze(Expression.analyze, nested_scope, params, exprs);

    Resolver.of_state_method(name, (res, snd(promise))) |> scope.resolve;

  | `Getter(params, exprs) =>
    let nested_scope =
      scope.nest(~label=Printf.sprintf("get(%s)", name), ());

    Function.analyze(Expression.analyze, nested_scope, params, exprs);
  };
