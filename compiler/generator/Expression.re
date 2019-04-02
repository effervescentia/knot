open Core;

let rec generate =
  fun
  | EqualsExpr(lhs, rhs) => gen_binary("===", lhs, rhs)
  | AddExpr(lhs, rhs) => gen_binary("+", lhs, rhs)
  | SubExpr(lhs, rhs) => gen_binary("-", lhs, rhs)
  | MulExpr(lhs, rhs) => gen_binary("*", lhs, rhs)
  | DivExpr(lhs, rhs) => gen_binary("/", lhs, rhs)
  | LTExpr(lhs, rhs) => gen_binary("<", lhs, rhs)
  | LTEExpr(lhs, rhs) => gen_binary("<=", lhs, rhs)
  | GTExpr(lhs, rhs) => gen_binary(">", lhs, rhs)
  | GTEExpr(lhs, rhs) => gen_binary(">=", lhs, rhs)
  | AndExpr(lhs, rhs) => gen_binary("&&", lhs, rhs)
  | OrExpr(lhs, rhs) => gen_binary("||", lhs, rhs)
  | TernaryExpr((pred, _), (if_expr, _), (else_expr, _)) =>
    Printf.sprintf(
      "%s?%s:%s",
      generate(pred),
      generate(if_expr),
      generate(else_expr),
    )
  | Reference(reference) => fst(reference) |> Reference.generate(generate)
  | JSX(jsx) => JSX.generate(generate, jsx)
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => gen_string(s)
  | FunctionLit(params, exprs) =>
    Function.gen_body(generate, params, exprs)
    |> Printf.sprintf("function %s")
and gen_binary = (divider, (lhs, _), (rhs, _)) =>
  Printf.sprintf("(%s%s%s)", generate(lhs), divider, generate(rhs));
