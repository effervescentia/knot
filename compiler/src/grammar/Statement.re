open Kore;

let variable = expr =>
  Keyword.let_
  >> Operator.assign(M.identifier >|= Block.value, expr)
  >|= AST.of_var;

let expression = expr => expr >|= AST.of_expr;

let parser = expr =>
  choice([variable(expr), expression(expr)]) |> M.terminated;
