open Globals;

let decl = parse_expr =>
  M.let_
  >> M.identifier
  >>= (
    name =>
      M.assign
      >> parse_expr
      ==> no_ctx
      % (expr => VariableDeclaration(name, expr))
  )
  |> M.terminated;

let assign = parse_expr =>
  Reference.refr(parse_expr)
  >>= (
    refr =>
      M.assign
      >> parse_expr
      ==> (expr => VariableAssignment(no_ctx(refr), no_ctx(expr)))
  )
  |> M.terminated;
