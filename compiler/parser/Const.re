open Core;

module M = Matchers;

let decl = input =>
  (
    M.decl(M.const)
    >>= (
      s =>
        M.assign
        >> Expression.expr
        >> return(ConstDecl(s))
        << optional(M.semicolon)
    )
  )(
    input,
  );