open Core;
open Scope;

let analyze_prop = (analyze_expr, scope, (name, expr)) => (
  name,
  analyze_expr(scope, expr),
);

let rec analyze = (analyze_expr, scope) =>
  (
    fun
    | Element(name, props, children) =>
      A_Element(
        name,
        List.map(analyze_prop(analyze_expr, scope), props),
        List.map(analyze(analyze_expr, scope), children),
      )
    | Fragment(children) =>
      A_Fragment(List.map(analyze(analyze_expr, scope), children))
    | TextNode(s) => A_TextNode(s)
    | EvalNode(expr) => A_EvalNode(analyze_expr(scope, expr))
  )
  % await_ctx
  % (
    x => {
      Resolver.of_jsx(x) |> scope.resolve;

      x;
    }
  );