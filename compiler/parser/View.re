open Core;
open AST;

module M = Matchers;

let rec decl = input =>
  (
    M.view
    >> M.identifier
    >>= (
      s =>
        _inheritance
        |= None
        >>= (
          inh =>
            _mixins
            |= []
            >>= (
              m =>
                Expression.expr
                >> return(ViewDecl(s))
                << optional(M.semicolon)
            )
        )
    )
  )(
    input,
  )
and _inheritance = M.colon >> M.identifier ==> (name => Some(name))
and _mixins = M.tilde >> comma_separated(M.identifier);