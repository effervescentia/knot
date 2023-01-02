open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Common.identifier_t, Expression.expression_t(unit))
  ) =>
  (Result.raw_statement_t, Type.t) =
  (scope, analyze_expression, (name, expression)) => {
    let expression' = analyze_expression(scope, expression);

    scope
    |> Scope.define(fst(name), Node.get_type(expression'))
    |> Option.iter(Scope.report_type_err(scope, Node.get_range(name)));

    ((name, expression') |> Result.of_var, Type.Valid(Nil));
  };
