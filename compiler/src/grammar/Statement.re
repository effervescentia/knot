open Kore;

let variable = expr =>
  Keyword.let_
  >> M.binary_op(M.identifier >|= fst, Operator.assign, expr)
  >|= AST.of_var;

let expression = expr => expr >|= AST.of_expr;

let parser = expr =>
  choice([variable(expr), expression(expr)]) |> M.terminated;
