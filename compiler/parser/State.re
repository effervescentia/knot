open Core;

module M = Matchers;

let prop_stmt = input =>
  (
    Property.prop
    ==> (((name, (type_def, expr))) => Property(name, type_def, expr))
  )(
    input,
  );
let mut_stmt = input =>
  (
    M.decl(M.mut)
    >>= (
      name =>
        Parameter.params
        |= []
        >>= (
          params => Function.body ==> (exprs => Mutator(name, params, exprs))
        )
    )
  )(
    input,
  );
let get_stmt = input =>
  (
    M.decl(M.get)
    >>= (
      name =>
        Parameter.params
        |= []
        >>= (
          params => Function.body ==> (exprs => Getter(name, params, exprs))
        )
    )
  )(
    input,
  );

let stmt = input => (mut_stmt <|> get_stmt <|> prop_stmt)(input);
let stmts = input => many1(stmt, input);

let decl = input =>
  (
    M.decl(M.state)
    >>= (
      s =>
        M.assign
        >> Expression.expr
        >> return(StateDecl(s))
        << optional(M.semicolon)
    )
  )(
    input,
  );