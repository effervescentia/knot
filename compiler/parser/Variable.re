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

let assign =
  Reference.refr(Expression.expr)
  >>= (
    refr =>
      M.assign
      >> Expression.expr
      ==> (expr => VariableAssignment(no_ctx(refr), no_ctx(expr)))
  )
  |> M.terminated;
