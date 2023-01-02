open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Expression.expression_t(unit)
  ) =>
  (Result.raw_statement_t, Type.t) =
  (scope, analyze_expression, expression) => {
    let expression' = analyze_expression(scope, expression);

    (Result.of_effect(expression'), Node.get_type(expression'));
  };
