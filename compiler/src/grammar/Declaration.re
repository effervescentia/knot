open Kore;

module TypeResolver = {
  open AST;

  let _bind_lexeme = (cursor: Cursor.t, x) => (x, cursor);
  let _bind_typed_lexeme = (cursor: Cursor.t, (x, y)) => (x, y, cursor);

  let res_prim = ((prim, type_, cursor): Raw.primitive_t): primitive_t =>
    Type2.Result.(
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
    | Variable(id, expr) => Variable(id, res_expr(expr))
    | Expression(expr) => Expression(res_expr(expr))
    }

  and res_expr = ((expr, type_, cursor): Raw.expression_t): expression_t =>
    (
      switch (expr) {
      | Primitive(prim) =>
        res_prim(prim) |> (x => (Primitive(x), Tuple.snd3(x)))
      | JSX(jsx) => (JSX(res_jsx(jsx)), Valid(`Element))
      | Group(expr) => res_expr(expr) |> (x => (Group(x), Tuple.snd3(x)))
      | Closure(stmts) =>
        stmts
        |> List.map(res_stmt)
        |> (
          xs => (
            Closure(xs),
            xs |> List.last |> Option.map(TypeOf.statement) |?: Valid(`Nil),
          )
        )
      | Identifier(id) => Identifier(id)
      }
    )
    |> _bind_typed_lexeme(cursor)

  and res_jsx = ((jsx, cursor): Raw.jsx_t): jsx_t =>
    (
      switch (jsx) {
      | Tag(id, attrs, children) =>
        Tag(
          id,
          attrs |> List.map(res_attr),
          children |> List.map(res_child),
        )
      | Fragment(children) => Fragment(children |> List.map(res_child))
      }
    )
    |> _bind_lexeme(cursor)

  and res_attr = ((attr, cursor): Raw.jsx_attribute_t): jsx_attribute_t =>
    (
      switch (attr) {
      | ID(id) => ID(id)
      | Class(id, expr) => Class(id, expr |> Option.map(res_expr))
      | Property(id, expr) => Property(id, expr |> Option.map(res_expr))
      }
    )
    |> _bind_lexeme(cursor)

  and res_child = ((attr, cursor): Raw.jsx_child_t): jsx_child_t =>
    (
      switch (attr) {
      | Text(text) => Text(text)
      | Node(jsx) => Node(res_jsx(jsx))
      | InlineExpression(expr) => InlineExpression(res_expr(expr))
      }
    )
    |> _bind_lexeme(cursor);
};

let constant = (ctx: ModuleContext.t, f) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  Keyword.const
  >> Operator.assign(
       Identifier.parser(closure_ctx),
       Expression.parser(closure_ctx),
     )
  >|= (
    (((id, _) as x, expr)) => {
      closure_ctx |> ClosureContext.define(id, TypeOf.lexeme(expr));

      (f(x), decl);
    }
  )
  |> M.terminated;
};

let function_ = (ctx: ModuleContext.t, f) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  Keyword.func
  >> Identifier.parser(closure_ctx)
  >>= (
    id => {
      let child_ctx = ClosureContext.child(closure_ctx);

      Lambda.parser(child_ctx)
      >|= (
        ((args, (_, _, expr_cursor) as expr)) => {
          let ctx_cursor = Cursor.join(expr_cursor, expr_cursor);

          child_ctx |> ClosureContext.save(ctx_cursor);

          (f(id), (args, expr) |> AST.Raw.of_func);
        }
      );
    }
  )
  |> M.terminated;
};

let parser = (ctx: ModuleContext.t) =>
  option(AST.of_named_export, AST.of_main_export <$ Keyword.main)
  >>= (f => choice([constant(ctx, f), function_(ctx, f)]) >|= AST.of_decl);
