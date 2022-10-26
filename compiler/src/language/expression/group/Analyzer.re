open Knot.Kore;

let analyze_group:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.expression_t
  ) =>
  AST.expression_t =
  (scope, analyze_expression, node) => analyze_expression(scope, node);
