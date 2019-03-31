open Core;

let expr_stmt =
  Expression.expr |> M.terminated ==> no_ctx % (x => ExpressionStatement(x));
let lambda = Variable.assign <|> expr_stmt ==> (x => [x]);
let closure = Variable.assign <|> expr_stmt <|> Variable.decl |> M.closure;
let body = M.lambda >> (closure <|> lambda ==> List.map(no_ctx));
let expr = input =>
  (Property.list >>= (params => body ==> (exprs => (params, exprs))))(input);

let decl =
  M.decl(M.func)
  >>= (
    name =>
      expr ==> (((params, exprs)) => (name, FunctionDecl(params, exprs)))
  );
