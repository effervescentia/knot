open Core;

let prop_stmt =
  Property.prop
  ==> (
    ((name, type_def, default_value)) => (
      name,
      no_ctx(`Property((type_def, default_value))),
    )
  )
  |> M.terminated;
let mut_stmt =
  M.decl(M.mut)
  >>= (
    name =>
      Property.list
      |= []
      >>= (
        params =>
          Function.body
          ==> (exprs => (name, no_ctx(`Mutator((params, exprs)))))
      )
  );
let get_stmt =
  M.decl(M.get)
  >>= (
    name =>
      Property.list
      |= []
      >>= (
        params =>
          Function.body
          ==> (exprs => (name, no_ctx(`Getter((params, exprs)))))
      )
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
          stmt |> M.closure ==> (stmts => (name, StateDecl(params, stmts)))
      )
  );
