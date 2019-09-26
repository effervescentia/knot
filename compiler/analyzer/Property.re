open Core;
open Scope;

let analyze_type_def = (scope, ast) =>
  ast |*> (type_def => Resolver.of_type(type_def) |> scope.resolve);

let analyze_default_value = (analyze_expr, expr) => expr |*> analyze_expr;

let analyze = (~resolve=true, analyze_expr, scope, prop) => {
  let (_, type_def, default_val) = fst(prop);

  analyze_type_def(scope, type_def);
  analyze_default_value(analyze_expr(scope), default_val);

  if (resolve) {
    Resolver.of_property(prop) |> scope.resolve;
  };
};

let analyze_param = (analyze_expr, scope, prop) => {
  analyze(~resolve=false, analyze_expr, scope, prop);

  Resolver.of_parameter(prop) |> scope.resolve;
};
