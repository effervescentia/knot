open Kore;

let constant = (ctx: ClosureContext.t, f) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser(ctx),
       Expression.parser(ctx) >|= AST.of_const,
     )
  >|= (
    (((id, _) as x, decl)) => {
      ctx |> ClosureContext.define(id, TypeOf.declaration(decl));

      (f(x), decl);
    }
  )
  |> M.terminated;

let function_ = (ctx: ClosureContext.t, f) =>
  Keyword.func
  >> Identifier.parser(ctx)
  >>= (
    id => {
      let child_ctx = ClosureContext.child(ctx);

      Lambda.parser(child_ctx)
      >|= (
        ((args, (_, _, expr_cursor) as expr)) => {
          let ctx_cursor = Cursor.join(expr_cursor, expr_cursor);

          child_ctx |> ClosureContext.save(ctx_cursor);

          (f(id), (args, expr) |> AST.of_func);
        }
      );
    }
  )
  |> M.terminated;

let parser = (ctx: ModuleContext.t) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  option(AST.of_named_export, AST.of_main_export <$ Keyword.main)
  >>= (
    f =>
      choice([constant(closure_ctx, f), function_(closure_ctx, f)])
      >|= AST.of_decl
  );
};
