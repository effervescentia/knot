open Kore;

let variable = (ctx: Context.t, expr) =>
  Keyword.let_
  >> Operator.assign(Identifier.parser(ctx), expr(ctx))
  >@= ((((id, _), (_, t, _))) => ctx.scope |> Scope.define(id, t))
  >|= AST.of_var;

let expression = (ctx: Context.t, expr) => expr(ctx) >|= AST.of_expr;

let parser = (ctx: Context.t, expr) =>
  choice([variable(ctx, expr), expression(ctx, expr)]) |> M.terminated;
