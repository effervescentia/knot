open Core;
open Scope;

exception InvalidTypeReference;

let analyze_type_def =
  fun
  | Some(type_def) =>
    (
      switch (type_def) {
      | "string" => String_t
      | "number" => Number_t
      | "boolean" => Number_t
      | _ => raise(InvalidTypeReference)
      }
    )
    |> (x => Some(x))
  | None => None;

let analyze =
    (~resolve=true, analyze_expr, scope, (name, type_def, default_val)) =>
  (
    name,
    analyze_type_def(type_def),
    opt_transform(analyze_expr(scope), default_val),
  )
  |> await_ctx
  |> (
    wrapped => {
      if (resolve) {
        Resolver.of_property(wrapped) |> scope.resolve;
      };

      wrapped;
    }
  );
