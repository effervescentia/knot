open Core;

let decl =
  M.decl(M.const)
  >>= (
    s => M.assign >> Expression.expr ==> no_ctx % (expr => ConstDecl(s, expr))
  )
  |> M.terminated;
