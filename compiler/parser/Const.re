open Core;

let decl =
  M.decl(M.const)
  >>= (s => M.assign >> Expression.expr ==> (expr => ConstDecl(s, expr)))
  |> M.terminated;