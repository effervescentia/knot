open Core;

let decl =
  M.decl(M.const)
  >>= (
    name =>
      M.assign
      >> Expression.expr
      ==> no_ctx
      % (expr => (name, ConstDecl(expr)))
  )
  |> M.terminated;
