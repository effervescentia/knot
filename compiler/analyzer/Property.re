open Core;
open Scope;

let analyze_type_def = scope =>
  fun
  | Some(type_def) => Resolver.of_type(type_def) |> scope.resolve
  | None => ();

let analyze = (~resolve=true, analyze_expr, scope, prop) => {
  let (_, type_def, default_val) = abandon_ctx(prop);

  analyze_type_def(scope, type_def);
  opt_transform(analyze_expr(scope), default_val) |> ignore;

  if (resolve) {
    Resolver.of_property(prop) |> scope.resolve;
  };
};
