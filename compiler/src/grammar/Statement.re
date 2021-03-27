open Kore;

let variable = (scope: Scope.t, expr) =>
  Keyword.let_
  >> Operator.assign(Identifier.parser, expr(scope))
  >@= ((((id, _), (_, t, _))) => scope |> Scope.define(id, t))
  >|= AST.of_var;

let expression = (scope: Scope.t, expr) => expr(scope) >|= AST.of_expr;

let parser = (scope: Scope.t, expr) =>
  choice([variable(scope, expr), expression(scope, expr)]) |> M.terminated;
