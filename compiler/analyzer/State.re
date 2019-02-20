open Core;

let analyze_prop = scope =>
  (
    fun
    | Property((name, type_def, default_val)) =>
      A_Property((
        name,
        type_def,
        opt_transform(Expression.analyze(scope), default_val),
      ))
    | Mutator(name, params, exprs) =>
      A_Mutator(
        name,
        List.map(Property.analyze(scope), params),
        List.map(Expression.analyze(scope), exprs),
      )
    | Getter(name, params, exprs) =>
      A_Getter(
        name,
        List.map(Property.analyze(scope), params),
        List.map(Expression.analyze(scope), exprs),
      )
  )
  % await_ctx;