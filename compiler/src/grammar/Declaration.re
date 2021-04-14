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

let parser = (ctx: Context.t) =>
  option(AST.of_named_export, AST.of_main_export <$ Keyword.main)
  >>= (f => choice([constant(ctx, f)]) >|= AST.of_decl);
