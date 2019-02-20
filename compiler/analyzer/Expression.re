open Core;
open Scope;

let rec analyze = scope =>
  (
    fun
    | AddExpr(lhs, rhs) =>
      A_AddExpr(analyze(scope, lhs), analyze(scope, rhs))
    | SubExpr(lhs, rhs) =>
      A_SubExpr(analyze(scope, lhs), analyze(scope, rhs))
    | MulExpr(lhs, rhs) =>
      A_MulExpr(analyze(scope, lhs), analyze(scope, rhs))
    | DivExpr(lhs, rhs) =>
      A_DivExpr(analyze(scope, lhs), analyze(scope, rhs))
    | LTExpr(lhs, rhs) =>
      A_LTExpr(analyze(scope, lhs), analyze(scope, rhs))
    | GTExpr(lhs, rhs) =>
      A_GTExpr(analyze(scope, lhs), analyze(scope, rhs))
    | LTEExpr(lhs, rhs) =>
      A_LTEExpr(analyze(scope, lhs), analyze(scope, rhs))
    | GTEExpr(lhs, rhs) =>
      A_GTEExpr(analyze(scope, lhs), analyze(scope, rhs))
    | AndExpr(lhs, rhs) =>
      A_AndExpr(analyze(scope, lhs), analyze(scope, rhs))
    | OrExpr(lhs, rhs) =>
      A_OrExpr(analyze(scope, lhs), analyze(scope, rhs))
    | Reference(x) => A_Reference(Reference.analyze(analyze, scope, x))
    | JSX(x) => A_JSX(JSX.analyze(analyze, scope, x))
    | NumericLit(x) => A_NumericLit(x)
    | BooleanLit(x) => A_BooleanLit(x)
    | StringLit(x) => A_StringLit(x)
  )
  % await_ctx
  % (
    x => {
      Resolver.of_expression(x) |> scope.resolve;

      x;
    }
  );