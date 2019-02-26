open Core;

let decl =
  Const.decl <|> State.decl <|> Function.decl <|> View.decl <|> Style.decl;
let main_decl = M.main >> decl ==> (d => Main(d));
let stmt = Import.stmt <|> main_decl <|> (decl ==> (d => Declaration(d)));

let stmts = many(stmt) ==> (l => Module(l));
