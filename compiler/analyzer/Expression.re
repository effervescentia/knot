open Core;
open Scope;

let rec analyze = (scope, expr) => {
  fst(expr)
  |> (
    fun
    | AddExpr(lhs, rhs)
    | SubExpr(lhs, rhs)
    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs)
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs)
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) => {
        analyze(scope, lhs);
        analyze(scope, rhs);
      }
    | TernaryExpr(pred, if_expr, else_expr) => {
        analyze(scope, pred);
        analyze(scope, if_expr);
        analyze(scope, else_expr);
      }
    | Reference(x) => Reference.analyze(analyze, scope, x)
    | JSX(x) => JSX.analyze(analyze, scope, x)
    | NumericLit(_)
    | BooleanLit(_)
    | StringLit(_) => ()
  );

  Resolver.of_expression(expr) |> scope.resolve;
};
