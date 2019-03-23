open Core;

let decl =
  M.let_
  >> M.identifier
  >>= (
    name =>
      M.assign
      >> Expression.expr
      ==> no_ctx
      % (expr => VariableDeclaration(name, expr))
  )
  |> M.terminated;
