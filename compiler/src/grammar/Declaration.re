open Kore;

let constant = (ctx: Context.t) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser,
       Expression.parser(ctx) >|= AST.of_const,
     )
  >@= (
    (((id, _), decl)) =>
      ctx.scope |> Scope.define(id, decl |> TypeOf.declaration)
  )
  |> M.terminated;

let parser = (ctx: Context.t) => choice([constant(ctx)]) >|= AST.of_decl;
