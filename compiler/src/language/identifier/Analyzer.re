open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let analyze_identifier: (Scope.t, string, Range.t) => Type.t =
  (scope, id, range) => {
    let type_opt = scope |> Scope.lookup(id);

    let type_ =
      type_opt
      |!: (
        () => {
          let err = Type.NotFound(id);

          err |> Scope.report_type_err(scope, range);

          Type.Invalid(NotInferrable);
        }
      );

    type_;
  };
