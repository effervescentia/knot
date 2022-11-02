open Knot.Kore;

module Scope = AST.Scope;

let analyze_group:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    AST.Raw.expression_t
  ) =>
  AST.Result.expression_t =
  (scope, analyze_expression, node) => analyze_expression(scope, node);
