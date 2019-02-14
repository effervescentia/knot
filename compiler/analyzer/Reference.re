open Core;

let rec analyze = (analyze_expr, scope) =>
  fun
  | Variable(name) => A_Variable(name)
  | DotAccess(lhs, rhs) =>
    A_DotAccess(
      analyze(analyze_expr, scope, lhs) |> wrap,
      analyze(analyze_expr, scope, rhs) |> wrap,
    )
  | Execution(target, args) =>
    A_Execution(
      analyze(analyze_expr, scope, target) |> wrap,
      analyze_list(analyze_expr(scope), args),
    );