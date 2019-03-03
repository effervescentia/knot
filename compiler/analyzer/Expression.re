open Core;
open Scope;

let rec analyze = (scope, expr) => {
  abandon_ctx(expr)
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
    | Reference(x) => Reference.analyze(analyze, scope, x)
    | JSX(x) => JSX.analyze(analyze, scope, x)
    /* | NumericLit(x) => A_NumericLit(x)
       | BooleanLit(x) => A_BooleanLit(x)
       | StringLit(x) => A_StringLit(x) */
    | _ => ()
  );

  Resolver.of_expression(expr) |> scope.resolve;
};
