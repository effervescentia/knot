open Kore;
open AST;

module TypeResolver = {
  let _bind_lexeme = (cursor: Cursor.t, x) => (x, cursor);
  let _bind_typed_lexeme = (cursor: Cursor.t, (x, y)) => (x, y, cursor);

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
    |> _bind_typed_lexeme(cursor);

  let rec res_stmt = (stmt: Raw.statement_t): statement_t =>
    switch (stmt) {
    | Variable(id, expr) => res_expr(expr) |> (x => Variable(id, x))
    | Expression(expr) => Expression(res_expr(expr))
    }

  and res_expr = ((expr, cursor): Raw.expression_t): expression_t =>
    (
      switch (expr) {
      | Primitive(prim) =>
        res_prim(prim) |> (x => (Primitive(x), Node.type_(x)))
      | JSX(jsx) => (JSX(res_jsx(jsx)), Valid(`Element))
      | Group(expr) => res_expr(expr) |> (x => (Group(x), Node.type_(x)))
      | Closure(stmts) =>
        stmts
        |> List.map(res_stmt)
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
          UnaryOp(op, res_expr(expr)),
          /* TODO: implement */
          Valid(`Abstract(Unknown)),
        )
      | BinaryOp(op, lhs, rhs) => (
          BinaryOp(op, res_expr(lhs), res_expr(rhs)),
          /* TODO: implement */
          Valid(`Abstract(Unknown)),
        )
      }
    )
    |> _bind_typed_lexeme(cursor)

  and res_jsx = ((jsx, cursor): Raw.jsx_t): jsx_t =>
    (
      switch (jsx) {
      | Tag(id, attrs, children) => (
          Tag(
            id,
            attrs |> List.map(res_attr),
            children |> List.map(res_child),
          ),
          Type.Valid(`Element),
        )
      | Fragment(children) => (
          Fragment(children |> List.map(res_child)),
          Type.Valid(`Element),
        )
      }
    )
    |> _bind_typed_lexeme(cursor)

  and res_attr = ((attr, cursor): Raw.jsx_attribute_t): jsx_attribute_t =>
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
            x |> Option.map(Node.type_) |?: Type.Valid(`Abstract(Unknown)),
          )
        )
      }
    )
    |> _bind_typed_lexeme(cursor)

  and res_child = ((attr, cursor): Raw.jsx_child_t): jsx_child_t =>
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
      | Node(jsx) => (Node(res_jsx(jsx)), Type.Valid(`Element))
      | InlineExpression(expr) =>
        res_expr(expr) |> (x => (InlineExpression(x), Node.type_(x)))
      }
    )
    |> _bind_typed_lexeme(cursor);
};

let constant = (ctx: ModuleContext.t, f) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  Keyword.const
  >>= (
    start =>
      Operator.assign(
        Identifier.parser(closure_ctx),
        Expression.parser(closure_ctx),
      )
      >|= (
        ((id, raw_expr)) => {
          let expr = TypeResolver.res_expr(raw_expr);

          /* ctx |> ModuleContext.define(Node.Raw.value(id), TypeOf.lexeme(expr)); */

          (
            f(id),
            (
              of_const(expr),
              Node.type_(expr),
              Cursor.join(Node.Raw.cursor(start), Node.cursor(expr)),
            ),
          );
        }
      )
      |> M.terminated
  );
};

let function_ = (ctx: ModuleContext.t, f) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  Keyword.func
  >>= (
    start =>
      Identifier.parser(closure_ctx)
      >>= (
        id => {
          let child_ctx = ClosureContext.child(closure_ctx);

          Lambda.parser(child_ctx)
          >|= (
            ((raw_args, raw_res)) => {
              let ctx_cursor =
                Cursor.join(
                  Node.Raw.cursor(raw_res),
                  Node.Raw.cursor(raw_res),
                );
              let res = TypeResolver.res_expr(raw_res);
              let args =
                raw_args
                |> List.map(((arg, cursor): Raw.argument_t) =>
                     (
                       {
                         name: arg.name,
                         default:
                           arg.default |> Option.map(TypeResolver.res_expr),
                         type_: None,
                       },
                       /* TODO: implement */
                       Type.Valid(`Abstract(Unknown)),
                       cursor,
                     )
                   );

              child_ctx |> ClosureContext.save(ctx_cursor);

              (
                f(id),
                (
                  (args, res) |> of_func,
                  Type.Valid(
                    `Function((
                      args
                      |> List.map((({name}, type_, _)) =>
                           (
                             name
                             |> Node.Raw.value
                             |> Reference.Identifier.to_string,
                             type_,
                           )
                         ),
                      Node.type_(res),
                    )),
                  ),
                  Cursor.join(Node.Raw.cursor(start), Node.cursor(res)),
                ),
              );
            }
          );
        }
      )
  )
  |> M.terminated;
};

let parser = (ctx: ModuleContext.t) =>
  option(of_named_export, of_main_export <$ Keyword.main)
  >>= (f => choice([constant(ctx, f), function_(ctx, f)]) >|= of_decl);
