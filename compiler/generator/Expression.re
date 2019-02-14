open Core;

let rec generate =
  fun
  | A_AddExpr(lhs, rhs) => gen_binary("+", lhs, rhs)
  | A_SubExpr(lhs, rhs) => gen_binary("-", lhs, rhs)
  | A_MulExpr(lhs, rhs) => gen_binary("*", lhs, rhs)
  | A_DivExpr(lhs, rhs) => gen_binary("/", lhs, rhs)
  | A_LTExpr(lhs, rhs) => gen_binary("<", lhs, rhs)
  | A_LTEExpr(lhs, rhs) => gen_binary("<=", lhs, rhs)
  | A_GTExpr(lhs, rhs) => gen_binary(">", lhs, rhs)
  | A_GTEExpr(lhs, rhs) => gen_binary(">=", lhs, rhs)
  | A_AndExpr(lhs, rhs) => gen_binary("&&", lhs, rhs)
  | A_OrExpr(lhs, rhs) => gen_binary("||", lhs, rhs)
  | A_Reference(reference) =>
    unwrap(reference) |> Reference.generate(generate)
  | A_JSX(jsx) => unwrap(jsx) |> JSX.generate(generate)
  | A_NumericLit(n) => string_of_int(n)
  | A_BooleanLit(b) => string_of_bool(b)
  | A_StringLit(s) => gen_string(s)
and gen_binary = (divider, lhs, rhs) =>
  Printf.sprintf(
    "(%s%s%s)",
    unwrap(lhs) |> generate,
    divider,
    unwrap(rhs) |> generate,
  );