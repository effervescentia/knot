open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Common.identifier_t, Expression.expression_t(unit))
  ) =>
  (Result.raw_statement_t, Type.t) =
  (scope, analyze_expression, (id, expr)) => {
    let expr' = analyze_expression(scope, expr);

    scope
    |> Scope.define(fst(id), Node.get_type(expr'))
    |> Option.iter(Scope.report_type_err(scope, Node.get_range(id)));

    ((id, expr') |> Result.of_var, Type.Valid(`Nil));
  };
