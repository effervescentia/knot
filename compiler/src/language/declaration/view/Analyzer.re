open Knot.Kore;
open AST;

let analyze_view_body:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.expression_t
  ) =>
  Result.expression_t =
  (scope, analyze_expression, body) => {
    let body' = body |> analyze_expression(scope);
    let type_ = Node.get_type(body');

    type_
    |> Validator.validate_jsx_primitive_expression
    |> Option.iter(body' |> Node.get_range |> Scope.report_type_err(scope));

    body';
  };
