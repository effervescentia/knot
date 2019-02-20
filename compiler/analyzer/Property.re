open Core;

let analyze = (scope, (name, type_def, default_val)) =>
  (name, type_def, opt_transform(Expression.analyze(scope), default_val))
  |> await_ctx;