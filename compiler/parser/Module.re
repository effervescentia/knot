open Globals;

let decl =
  Const.decl
  <|> State.decl
  <|> Function.decl(Expression.expr)
  <|> View.decl
  <|> Style.decl
  ==> (((name, decl)) => (name, no_ctx(decl)));
let main_decl =
  M.main >> decl ==> (((name, decl)) => ModuleStatement(Main(name, decl)));
let stmt =
  Import.stmt
  ==> (i => ModuleImport(i))
  <|> main_decl
  <|> (
    decl ==> (((name, decl)) => ModuleStatement(Declaration(name, decl)))
  );

let stmts =
  many(stmt)
  ==> (
    l => {
      let (imports, statements) =
        List.fold_left(
          ((imports, statements)) =>
            fun
            | ModuleStatement(x) => (imports, statements @ [x])
            | ModuleImport(x) => (imports @ [x], statements),
          ([], []),
          l,
        );

      Module(imports, statements);
    }
  );
