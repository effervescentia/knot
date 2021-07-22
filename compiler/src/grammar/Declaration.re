open Kore;

let constant = (ctx: Context.t, f) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser(ctx),
       Expression.parser(ctx) >|= AST.of_const,
     )
  >|= (
    (((id, _) as x, decl)) => {
      ctx.scope |> Scope.define(id, decl |> TypeOf.declaration);

      (f(x), decl);
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

          (f(id), (args, expr) |> AST.of_func);
        }
      );
    }
  )
  |> M.terminated;

let parser = (ctx: ModuleContext.t) =>
  option(AST.of_named_export, AST.of_main_export <$ Keyword.main)
  >>= (f => choice([constant(ctx, f), function_(ctx, f)]) >|= AST.of_decl);
