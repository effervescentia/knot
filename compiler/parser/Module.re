open Core;

let decl =
  Const.decl
  <|> State.decl
  <|> Function.decl
  <|> View.decl
  <|> Style.decl
  ==> (((name, decl)) => (name, no_ctx(decl)));
let main_decl = M.main >> decl ==> (((name, decl)) => Main(name, decl));
let stmt =
  Import.stmt
  <|> main_decl
  <|> (decl ==> (((name, decl)) => Declaration(name, decl)));

let stmts = many(stmt) ==> (l => Module(l));
