open Core;
open Scope;

exception InvalidTypeReference;

let analyze_type_def =
  fun
  | Some(type_def) =>
    (
      switch (abandon_ctx(type_def)) {
      | "string" => String_t
      | "number" => Number_t
      | "boolean" => Number_t
      | _ => raise(InvalidTypeReference)
      }
    )
    |> (x => Some(x))
  | None => None;

let analyze = (~resolve=true, analyze_expr, scope, prop) => {
  let (_, type_def, default_val) = abandon_ctx(prop);

  analyze_type_def(type_def) |> ignore;
  opt_transform(analyze_expr(scope), default_val) |> ignore;

  if (resolve) {
    Resolver.of_property(prop) |> scope.resolve;
  };
};
