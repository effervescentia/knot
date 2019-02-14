open Core;

let rec analyze = scope =>
  fun
  | AddExpr(lhs, rhs) =>
    A_AddExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | SubExpr(lhs, rhs) =>
    A_SubExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | MulExpr(lhs, rhs) =>
    A_MulExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | DivExpr(lhs, rhs) =>
    A_DivExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | LTExpr(lhs, rhs) =>
    A_LTExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | GTExpr(lhs, rhs) =>
    A_GTExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | LTEExpr(lhs, rhs) =>
    A_LTEExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | GTEExpr(lhs, rhs) =>
    A_GTEExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | AndExpr(lhs, rhs) =>
    A_AndExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | OrExpr(lhs, rhs) =>
    A_OrExpr(analyze(scope, lhs) |> wrap, analyze(scope, rhs) |> wrap)
  | Reference(x) => A_Reference(Reference.analyze(analyze, scope, x) |> wrap)
  | JSX(x) => A_JSX(JSX.analyze(scope, x) |> wrap)
  | NumericLit(x) => A_NumericLit(x)
  | BooleanLit(x) => A_BooleanLit(x)
  | StringLit(x) => A_StringLit(x);