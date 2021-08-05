open Kore;
open AST;

let _bind_node = (cursor: Cursor.t, x) => (x, cursor);
let _bind_typed_node = (cursor: Cursor.t, (x, y)) => (x, y, cursor);

let res_prim = ((prim, cursor): Raw.primitive_t): primitive_t =>
  Type.(
    switch (prim) {
    | Nil => (Nil, Valid(`Nil))
    | Boolean(bool) => (Boolean(bool), Valid(`Boolean))
    | Number(Integer(_) as int) => (Number(int), Valid(`Integer))
    | Number(Float(_) as float) => (Number(float), Valid(`Float))
    | String(str) => (String(str), Valid(`String))
    }
  )
  |> _bind_typed_node(cursor);

let rec res_stmt = (scope: Scope.t, stmt: Raw.statement_t): statement_t =>
  switch (stmt) {
  | Variable(id, expr) => res_expr(scope, expr) |> (x => Variable(id, x))
  | Expression(expr) => Expression(res_expr(scope, expr))
  }

and res_expr =
    (scope: Scope.t, (expr, cursor): Raw.expression_t): expression_t =>
  (
    switch (expr) {
    | Primitive(prim) =>
      res_prim(prim) |> (x => (Primitive(x), Node.type_(x)))
    | JSX(jsx) => (JSX(res_jsx(scope, jsx)), Valid(`Element))
    | Group(expr) =>
      res_expr(scope, expr) |> (x => (Group(x), Node.type_(x)))
    | Closure(stmts) =>
      stmts
      |> List.map(res_stmt(Scope.child(scope, cursor)))
      |> (
        xs => (
          Closure(xs),
          xs |> List.last |> Option.map(TypeOf.statement) |?: Valid(`Nil),
        )
      )
    | Identifier((id, cursor)) => (
        Identifier((id, Valid(`Abstract(Unknown)), cursor)),
        /* TODO: implement */
        Valid(`Abstract(Unknown)),
      )
    | UnaryOp(op, expr) => (
        UnaryOp(op, res_expr(scope, expr)),
        /* TODO: implement */
        Valid(`Abstract(Unknown)),
      )
    | BinaryOp(op, lhs, rhs) => (
        BinaryOp(op, res_expr(scope, lhs), res_expr(scope, rhs)),
        /* TODO: implement */
        Valid(`Abstract(Unknown)),
      )
    }
  )
  |> _bind_typed_node(cursor)

and res_jsx = (scope: Scope.t, (jsx, cursor): Raw.jsx_t): jsx_t =>
  (
    switch (jsx) {
    | Tag(id, attrs, children) => (
        Tag(
          id,
          attrs |> List.map(res_attr(scope)),
          children |> List.map(res_child(scope)),
        ),
        Type.Valid(`Element),
      )
    | Fragment(children) => (
        Fragment(children |> List.map(res_child(scope))),
        Type.Valid(`Element),
      )
    }
  )
  |> _bind_typed_node(cursor)

and res_attr =
    (scope: Scope.t, (attr, cursor): Raw.jsx_attribute_t): jsx_attribute_t =>
  (
    switch (attr) {
    | ID(id) => (ID(id), Type.Valid(`String))
    | Class(id, expr) => (
        Class(id, expr |> Option.map(res_expr(scope))),
        Type.Valid(`String),
      )
    | Property(id, expr) =>
      expr
      |> Option.map(res_expr(scope))
      |> (
        x => (
          Property(id, x),
          x |> Option.map(Node.type_) |?: Type.Valid(`Abstract(Unknown)),
        )
      )
    }
  )
  |> _bind_typed_node(cursor)

and res_child =
    (scope: Scope.t, (attr, cursor): Raw.jsx_child_t): jsx_child_t =>
  (
    switch (attr) {
    | Text(text) => (
        Text((
          Node.Raw.value(text),
          Type.Valid(`String),
          Node.Raw.cursor(text),
        )),
        Type.Valid(`String),
      )
    | Node(jsx) => (Node(res_jsx(scope, jsx)), Type.Valid(`Element))
    | InlineExpression(expr) =>
      res_expr(scope, expr) |> (x => (InlineExpression(x), Node.type_(x)))
    }
  )
  |> _bind_typed_node(cursor);
