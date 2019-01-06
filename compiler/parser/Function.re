open Core;
open AST;

module M = Matchers;

let rec decl = input =>
  (
    M.func
    >> M.identifier
    >>= (
      s =>
        M.assign
        >> Expression.expr
        >> return(FunctionDecl(s))
        << optional(M.semicolon)
    )
  )(
    input,
  )
/* and expr = input =>
   (Parameter.params >>= (params => body ==> (exprs => (params, exprs))))(
     input,
   ) */
and body = input => (M.lambda >> (closure <|> lambda))(input)
and closure = input =>
  (many(Expression.expr << optional(M.semicolon)) |> M.braces)(input)
and lambda = input =>
  (Expression.expr ==> (expr => [expr]) << optional(M.semicolon))(input);