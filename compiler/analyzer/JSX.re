open Core;

let analyze_prop = (analyze_expr, scope, (name, expr)) => (
  name,
  analyze_expr(expr) |> wrap,
);

let rec analyze = (analyze_expr, scope) =>
  fun
  | Element(name, props, children) =>
    A_Element(
      name,
      List.map(analyze_prop(analyze_expr(scope), scope), props),
      analyze_list(analyze(analyze_expr, scope), children),
    )
  | Fragment(children) =>
    A_Fragment(analyze_list(analyze(analyze_expr, scope), children))
  | TextNode(s) => A_TextNode(s)
  | EvalNode(expr) => A_EvalNode(analyze_expr(scope, expr) |> wrap);