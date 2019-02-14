open Core;

let analyze = (scope, (name, type_def, default_val)) => (
  name,
  type_def,
  wrap_and_trfm_opt(Expression.analyze(scope), default_val),
);