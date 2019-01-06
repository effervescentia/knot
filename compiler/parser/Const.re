open Core;
open AST;

module M = Matchers;

let decl = input =>
  (
    M.const
    >> M.identifier
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