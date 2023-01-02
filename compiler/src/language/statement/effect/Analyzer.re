open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Expression.expression_t(unit)
  ) =>
  (Result.raw_statement_t, Type.t) =
  (scope, analyze_expression, expr) => {
    let expr' = analyze_expression(scope, expr);

    (Result.of_effect(expr'), Node.get_type(expr'));
  };
