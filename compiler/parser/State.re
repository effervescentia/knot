open Core;

let prop_stmt =
  Property.prop ==> (property => Property(property)) |> M.terminated;
let mut_stmt =
  M.decl(M.mut)
  >>= (
    name =>
      Property.list
      |= []
      >>= (
        params => Function.body ==> (exprs => Mutator(name, params, exprs))
      )
  );
let get_stmt =
  M.decl(M.get)
  >>= (
    name =>
      Property.list
      |= []
      >>= (params => Function.body ==> (exprs => Getter(name, params, exprs)))
  );

let stmt = mut_stmt <|> get_stmt <|> prop_stmt;

let decl =
  M.decl(M.state)
  >>= (
    name =>
      Property.list
      |= []
      >>= (
        params =>
          M.closure(stmt) ==> (stmts => StateDecl(name, params, stmts))
      )
  );
