open Core;
open Scope;

let rec analyze = (analyze_expr, scope) =>
  (
    fun
    | Variable(name) => A_Variable(name)
    | DotAccess(lhs, rhs) =>
      A_DotAccess(analyze(analyze_expr, scope, lhs), rhs)
    | Execution(target, args) =>
      A_Execution(
        analyze(analyze_expr, scope, target),
        List.map(analyze_expr(scope), args),
      )
  )
  % await_ctx
  % (
    x => {
      Resolver.of_reference(x) |> scope.resolve;

      x;
    }
  );
