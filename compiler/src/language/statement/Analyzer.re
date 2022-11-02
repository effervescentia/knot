open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let analyze_statement:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    AST.Raw.statement_t
  ) =>
  AST.Result.statement_t =
  (scope, analyze_expression, node) =>
    (
      switch (node) {
      | (Variable(id, expr), _) =>
        let expr' = analyze_expression(scope, expr);

        scope
        |> Scope.define(fst(id), Node.get_type(expr'))
        |> Option.iter(Scope.report_type_err(scope, Node.get_range(id)));

        ((id, expr') |> AST.Result.of_var, Type.Valid(`Nil));

      | (Expression(expr), _) =>
        let expr' = analyze_expression(scope, expr);

        (AST.Result.of_expr(expr'), Node.get_type(expr'));
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, Node.get_range(node)));
