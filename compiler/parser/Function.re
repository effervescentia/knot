open Core;

module M = Matchers;

let closure = input =>
  (many(Expression.expr) |> M.terminated |> M.braces)(input);
let lambda = input =>
  (Expression.expr ==> (expr => [expr]) |> M.terminated)(input);
let body = input => (M.lambda >> (closure <|> lambda))(input);
let expr = input =>
  (Parameter.params >>= (params => body ==> (exprs => (params, exprs))))(
    input,
  );

let rec decl = input =>
  (
    M.decl(M.func)
    >>= (
      s =>
        M.assign
        >> Expression.expr
        >> return(FunctionDecl(s))
        << optional(M.semicolon)
    )
  )(
    input,
  );