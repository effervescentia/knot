open Kore;

let constant = (ctx: Context.t, f) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser(ctx),
       Expression.parser(ctx) >|= RawUtil.const,
     )
  >|= (
    (((id, _) as x, decl)) => {
      ctx.scope |> Scope.define(id, decl |> TypeOf.declaration);

      (f(x), decl);
    }
  )
  |> M.terminated;

let type_ = (ctx: Context.t, f) =>
  Keyword.type_
  >> Operator.assign(
       Identifier.parser(ctx),
       TypePrimitive.parser >|= (x => AST.Final.Util.type_(x)),
     )
  >|= (
    (((id, _) as x, decl)) => {
      (
        /* ctx.scope |> Scope.define(id, decl |> TypeOf.declaration); */
        f(x),
        decl,
      );
    }
  )
  |> M.terminated;

let function_ = (ctx: Context.t, f) =>
  Keyword.func
  >> Identifier.parser(ctx)
  >>= (
    id => {
      let child_ctx = ctx |> Context.child;

      Lambda.parser(child_ctx)
      >|= (
        ((args, (_, _, expr_cursor) as expr)) => {
          let ctx_cursor = Cursor.join(expr_cursor, expr_cursor);

          Context.submit(ctx_cursor, child_ctx);

          (f(id), (args, expr) |> RawUtil.func);
        }
      );
    }
  )
  |> M.terminated;

let parser = (ctx: Context.t) =>
  option(RawUtil.named_export, RawUtil.main_export <$ Keyword.main)
  >>= (
    f =>
      [constant, function_]
      |> List.map(x => x(ctx, f))
      |> choice
      >|= RawUtil.decl
  );
