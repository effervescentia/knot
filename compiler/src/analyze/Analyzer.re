open Kore;
open AST;

let _bind_node = (range: Range.t, x) => (x, range);
let _bind_typed_node = (range: Range.t, (x, y)) => (x, y, range);

let res_prim = ((prim, range): Raw.primitive_t): primitive_t =>
  Type.(
    switch (prim) {
    | Nil => (Nil, Valid(`Nil))
    | Boolean(bool) => (Boolean(bool), Valid(`Boolean))
    | Number(Integer(_) as int) => (Number(int), Valid(`Integer))
    | Number(Float(_) as float) => (Number(float), Valid(`Float))
    | String(str) => (String(str), Valid(`String))
    }
  )
  |> _bind_typed_node(range);

let rec res_stmt =
        (scope: Scope.t, (stmt, range): Raw.statement_t): statement_t =>
  (
    switch (stmt) {
    | Variable(id, expr) => (
        Variable(id, res_expr(scope, expr)),
        Type.Valid(`Nil),
      )

    | Expression(expr) =>
      res_expr(scope, expr) |> (x => (Expression(x), Node.get_type(x)))
    }
  )
  |> _bind_typed_node(range)

and res_expr =
    (scope: Scope.t, (expr, range): Raw.expression_t): expression_t =>
  (
    switch (expr) {
    | Primitive(prim) =>
      res_prim(prim) |> (x => (Primitive(x), Node.get_type(x)))

    | JSX(jsx) =>
      res_jsx(res_expr(scope), jsx) |> (x => (JSX(x), Node.get_type(x)))

    | Group(expr) =>
      res_expr(scope, expr) |> (x => (Group(x), Node.get_type(x)))

    | Closure(stmts) =>
      stmts
      |> List.map(res_stmt(Scope.child(scope, range)))
      |> (
        xs => (
          Closure(xs),
          xs
          |> List.last
          |> Option.map(Node.get_value % TypeOf.statement)
          |?: Valid(`Nil),
        )
      )

    | Identifier((id, range)) => (
        Identifier((id, Valid(`Abstract(Unknown)), range)),
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
  |> _bind_typed_node(range)

and res_jsx =
    (res_expr: Raw.expression_t => expression_t, (jsx, range): Raw.jsx_t)
    : jsx_t =>
  (
    switch (jsx) {
    | Tag(id, attrs, children) => (
        Tag(
          id,
          attrs |> List.map(res_attr(res_expr)),
          children |> List.map(res_child(res_expr)),
        ),
        Type.Valid(`Element),
      )
    | Fragment(children) => (
        Fragment(children |> List.map(res_child(res_expr))),
        Type.Valid(`Element),
      )
    }
  )
  |> _bind_typed_node(range)

and res_attr =
    (
      res_expr: Raw.expression_t => expression_t,
      (attr, range): Raw.jsx_attribute_t,
    )
    : jsx_attribute_t =>
  (
    switch (attr) {
    | ID(id) => (ID(id), Type.Valid(`String))

    | Class(id, expr) => (
        Class(id, expr |> Option.map(res_expr)),
        Type.Valid(`String),
      )

    | Property(id, expr) =>
      expr
      |> Option.map(res_expr)
      |> (
        x => (
          Property(id, x),
          x |> Option.map(Node.get_type) |?: Type.Valid(`Abstract(Unknown)),
        )
      )
    }
  )
  |> _bind_typed_node(range)

and res_child =
    (
      res_expr: Raw.expression_t => expression_t,
      (attr, range): Raw.jsx_child_t,
    )
    : jsx_child_t =>
  (
    switch (attr) {
    | Text(text) => (
        Text((
          Node.Raw.get_value(text),
          Type.Valid(`String),
          Node.Raw.get_range(text),
        )),
        Type.Valid(`String),
      )

    | Node(jsx) => (Node(res_jsx(res_expr, jsx)), Type.Valid(`Element))

    | InlineExpression(expr) =>
      res_expr(expr) |> (x => (InlineExpression(x), Node.get_type(x)))
    }
  )
  |> _bind_typed_node(range);

let res_constant =
    (res_expr: Raw.expression_t => expression_t, raw_expr: Raw.expression_t)
    : declaration_t => {
  let expr = res_expr(raw_expr);

  Node.create(of_const(expr), Node.get_type(expr), Node.get_range(expr));
};

let res_function =
    (
      res_expr: Raw.expression_t => expression_t,
      raw_args: list(Raw.argument_t),
      raw_res: Raw.expression_t,
      range: Range.t,
    ) => {
  let res = res_expr(raw_res);
  let args =
    raw_args
    |> List.map(((arg, range): Raw.argument_t) =>
         (
           {
             name: arg.name,
             default: arg.default |> Option.map(res_expr),
             type_: None,
           },
           /* TODO: implement */
           Type.Valid(`Abstract(Unknown)),
           range,
         )
       );

  Node.create(
    (args, res) |> of_func,
    Type.Valid(
      `Function((
        args
        |> List.map((({name}, type_, _)) =>
             (
               name |> Node.Raw.get_value |> Reference.Identifier.to_string,
               type_,
             )
           ),
        Node.get_type(res),
      )),
    ),
    range,
  );
};
