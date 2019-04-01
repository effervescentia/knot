open Core;

let expr_stmt = parse_expr =>
  parse_expr |> M.terminated ==> no_ctx % (x => ExpressionStatement(x));
let lambda = parse_expr =>
  Variable.assign(parse_expr) <|> expr_stmt(parse_expr) ==> (x => [x]);
let closure = parse_expr =>
  Variable.assign(parse_expr)
  <|> expr_stmt(parse_expr)
  <|> Variable.decl(parse_expr)
  |> M.closure;
let body = parse_expr =>
  M.lambda
  >> (closure(parse_expr) <|> lambda(parse_expr) ==> List.map(no_ctx));
let expr = (parse_expr, input) =>
  (
    Property.list(parse_expr)
    >>= (params => body(parse_expr) ==> (exprs => (params, exprs)))
  )(
    input,
  );

let decl = parse_expr =>
  M.decl(M.func)
  >>= (
    name =>
      expr(parse_expr)
      ==> (((params, exprs)) => (name, FunctionDecl(params, exprs)))
  );
