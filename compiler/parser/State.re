open Core;
open AST;

module M = Matchers;

let decl = input =>
  (
    M.state
    >> M.identifier
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