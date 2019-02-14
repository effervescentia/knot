open Core;

let analyze_prop = scope =>
  fun
  | Property((name, type_def, default_val)) =>
    A_Property((
      name,
      type_def,
      wrap_and_trfm_opt(Expression.analyze(scope), default_val),
    ))
  | Mutator(name, params, exprs) =>
    A_Mutator(
      name,
      analyze_list(Property.analyze(scope), params),
      analyze_list(Expression.analyze(scope), exprs),
    )
  | Getter(name, params, exprs) =>
    A_Getter(
      name,
      analyze_list(Property.analyze(scope), params),
      analyze_list(Expression.analyze(scope), exprs),
    );