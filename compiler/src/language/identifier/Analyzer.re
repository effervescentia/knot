open Knot.Kore;
open AST;

let analyze: (Scope.t, string, Range.t) => Type.t =
  (scope, name, range) => {
    let resolve_error = err => {
      err |> Scope.report_type_err(scope, range);
      Type.Invalid(NotInferrable);
    };

    let type_ =
      scope
      |> Scope.lookup(name)
      |> Option.fold(
           ~some=
             Stdlib.Result.fold(
               ~ok=Option.some,
               ~error=resolve_error % Option.some,
             ),
           ~none=None,
         )
      |!: (() => Type.NotFound(name) |> resolve_error);

    type_;
  };
