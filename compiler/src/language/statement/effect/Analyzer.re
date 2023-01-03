open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.expression_t
  ) =>
  (Result.raw_statement_t, Type.t) =
  (scope, analyze_expression, expression) => {
    let expression' = analyze_expression(scope, expression);

    (Result.of_effect(expression'), Node.get_type(expression'));
  };
