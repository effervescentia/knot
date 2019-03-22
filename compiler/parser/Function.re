open Core;

let lambda =
  Expression.expr
  ==> no_ctx
  % (expr => [ExpressionStatement(expr)])
  |> M.terminated;
let body =
  M.lambda
  >> (
    Expression.expr
    |> M.terminated
    ==> (
      expr =>
        ExpressionStatement(expr)
        <|> Variable.decl
        |> M.closure
        ==> List.map(no_ctx)
        <|> lambda
    )
  );
let expr = input =>
  (Property.list >>= (params => body ==> (exprs => (params, exprs))))(input);

let decl =
  M.decl(M.func)
  >>= (
    name =>
      expr ==> (((params, exprs)) => (name, FunctionDecl(params, exprs)))
  );
