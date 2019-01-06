open Core;

let import =
  M.import
  >> M.identifier
  >>= (
    main =>
      M.from >> M.string >>= (s => return(Import(s, [MainExport(main)])))
  )
  |> M.terminated;
let decl =
  Const.decl
  <|> State.decl
  <|> Function.decl
  <|> View.decl
  ==> (d => Declaration(d));
let stmt = import <|> decl;

let stmts = many(stmt) ==> (l => Statements(l));