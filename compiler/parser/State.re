open Core;

let prop_stmt =
  Property.prop
  ==> (((name, (type_def, expr))) => Property(name, type_def, expr));
let mut_stmt =
  M.decl(M.mut)
  >>= (
    name =>
      Parameter.params
      |= []
      >>= (
        params => Function.body ==> (exprs => Mutator(name, params, exprs))
      )
  );
let get_stmt =
  M.decl(M.get)
  >>= (
    name =>
      Parameter.params
      |= []
      >>= (params => Function.body ==> (exprs => Getter(name, params, exprs)))
  );

let stmt = mut_stmt <|> get_stmt <|> prop_stmt;
let stmts = many1(stmt);

let decl =
  M.decl(M.state)
  >>= (s => M.assign >> Expression.expr >> return(StateDecl(s)))
  |> M.terminated;