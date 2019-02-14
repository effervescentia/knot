open Core;

let lambda = Expression.expr ==> (expr => [expr]) |> M.terminated;
let body =
  M.lambda >> (Expression.expr |> M.terminated |> M.closure <|> lambda);
let expr = Property.list >>= (params => body ==> (exprs => (params, exprs)));

let decl =
  M.decl(M.func)
  >>= (
    name =>
      expr ==> (((params, exprs)) => FunctionDecl(name, params, exprs))
  );
