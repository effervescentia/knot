open Knot.Kore;
open AST;

let analyze_group:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.expression_t
  ) =>
  Result.expression_t =
  (scope, analyze_expression, node) => analyze_expression(scope, node);
