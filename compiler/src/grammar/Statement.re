open Kore;
open AST.Raw;

let variable = (ctx: ClosureContext.t, expr) =>
  Keyword.let_
  >> Operator.assign(Identifier.parser(ctx), expr(ctx))
  /* TODO: move to type inference module */
  /* >@= ((((id, _), (_, t, _))) => ctx |> ClosureContext.define(id, t)) */
  >|= of_var;

let expression = (ctx: ClosureContext.t, expr) => expr(ctx) >|= of_expr;

let parser = (ctx: ClosureContext.t, expr) =>
  choice([variable(ctx, expr), expression(ctx, expr)]) |> M.terminated;
