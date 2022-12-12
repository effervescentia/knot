open Knot.Kore;
open AST;

let analyze: (Scope.t, string, Range.t) => Type.t =
  (scope, id, range) => {
    let resolve_error = err => {
      err |> Scope.report_type_err(scope, range);
      Type.Invalid(NotInferrable);
    };

    let type_ =
      scope
      |> Scope.lookup(id)
      |> Option.fold(
           ~some=
             Stdlib.Result.fold(
               ~ok=Option.some,
               ~error=resolve_error % Option.some,
             ),
           ~none=None,
         )
      |!: (() => Type.NotFound(id) |> resolve_error);

    type_;
  };
