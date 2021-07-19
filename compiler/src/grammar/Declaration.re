open Kore;
open AST.Final.Util;

let constant = (ctx: Context.t, f) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser(ctx),
       Expression.parser(ctx) >|= to_const,
     )
  >|= (
    (((id, _) as x, decl)) => {
      ctx.scope |> Scope.define(id, TypeOf.declaration(decl));

      (f(x), decl);
    }
  )
  |> M.terminated;

let type_ = (ctx: Context.t, f) =>
  Keyword.type_
  >> Operator.assign(
       Identifier.parser(ctx),
       TypePrimitive.parser >|= AST.Final.Util.to_type,
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
      let child_ctx = Context.child(ctx);

      Lambda.parser(child_ctx)
      >|= (
        ((args, (_, _, expr_cursor) as expr)) => {
          let ctx_cursor = Cursor.join(expr_cursor, expr_cursor);

          Context.submit(ctx_cursor, child_ctx);

          (f(id), (args, expr) |> to_func);
        }
      );
    }
  )
  |> M.terminated;

let parser = (ctx: Context.t) =>
  option(to_named_export, to_main_export <$ Keyword.main)
  >>= (
    f =>
      [constant, function_] |> List.map(x => x(ctx, f)) |> choice >|= to_decl
  );
