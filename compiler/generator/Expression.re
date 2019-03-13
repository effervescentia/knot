open Core;

let rec generate =
  fun
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
  | Reference(reference) => fst(reference) |> Reference.generate(generate)
  | JSX(jsx) => JSX.generate(generate, jsx)
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => gen_string(s)
and gen_binary = (divider, (lhs, _), (rhs, _)) =>
  Printf.sprintf("(%s%s%s)", generate(lhs), divider, generate(rhs));
