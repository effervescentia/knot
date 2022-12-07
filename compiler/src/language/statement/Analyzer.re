open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.statement_t
  ) =>
  Result.statement_t =
  (scope, analyze_expression, node) =>
    (
      switch (node) {
      | (Variable(id, expr), _) =>
        let expr' = analyze_expression(scope, expr);

        scope
        |> Scope.define(fst(id), Node.get_type(expr'))
        |> Option.iter(Scope.report_type_err(scope, Node.get_range(id)));

        ((id, expr') |> Result.of_var, Type.Valid(`Nil));

      | (Expression(expr), _) =>
        let expr' = analyze_expression(scope, expr);

        (Result.of_expr(expr'), Node.get_type(expr'));
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, Node.get_range(node)));
