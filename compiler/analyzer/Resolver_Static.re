open Core;

let rec resolve =
  fun
  | ExpressionScope({contents: Pending(expr)} as promise) =>
    resolve_expr(promise, expr)
  | _ => false
and resolve_expr = promise =>
  fun
  | A_NumericLit(_) as expr => {
      promise := Resolved(expr, ());
      true;
    }
  | _ => false;
