open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let validate_jsx_primitive_expression: Type.t => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) => None

  | type_ => Some(InvalidJSXPrimitiveExpression(type_));

let analyze_view_body:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    AST.Raw.expression_t
  ) =>
  AST.Result.expression_t =
  (scope, analyze_expression, body) => {
    let body' = body |> analyze_expression(scope);
    let type_ = Node.get_type(body');

    type_
    |> validate_jsx_primitive_expression
    |> Option.iter(body' |> Node.get_range |> Scope.report_type_err(scope));

    body';
  };
