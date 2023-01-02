open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.expression_t
  ) =>
  Result.expression_t =
  (scope, analyze_expression, expression) =>
    analyze_expression(scope, expression);
