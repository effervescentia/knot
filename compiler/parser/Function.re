open Core;

let lambda =
  Expression.expr
  ==> no_ctx
  % (x => [ExpressionStatement(x)])
  |> M.terminated;
let body =
  M.lambda
  >> (
    Expression.expr
    |> M.terminated
    ==> no_ctx
    % (x => ExpressionStatement(x))
    <|> Variable.decl
    |> M.closure
    <|> lambda
    ==> List.map(no_ctx)
  );
let expr = input =>
  (Property.list >>= (params => body ==> (exprs => (params, exprs))))(input);

let decl =
  M.decl(M.func)
  >>= (
    name =>
      expr ==> (((params, exprs)) => (name, FunctionDecl(params, exprs)))
  );
