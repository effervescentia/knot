open Core;
open Scope;

let analyze_prop = (analyze_expr, scope, (_, expr)) =>
  analyze_expr(scope, expr);

let rec analyze = (analyze_expr, scope) =>
  fun
  | Element(_, _, props, children) => {
      List.iter(analyze_prop(analyze_expr, scope), props);
      List.iter(analyze(analyze_expr, scope), children);
    }
  | Fragment(children) => List.iter(analyze(analyze_expr, scope), children)
  | EvalNode(expr) => analyze_expr(scope, expr)
  | TextNode(_) => ();
