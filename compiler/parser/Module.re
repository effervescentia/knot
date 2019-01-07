open Core;

let decl =
  Const.decl
  <|> State.decl
  <|> Function.decl
  <|> View.decl
  <|> Style.decl
  ==> (d => Declaration(d));
let stmt = Import.stmt <|> decl;

let stmts = many(stmt) ==> (l => Statements(l));