open Kore;

let variable = (ctx: ClosureContext.t, expr) =>
  Keyword.let_
  >> Operator.assign(Identifier.parser(ctx), expr(ctx))
  >@= ((((id, _), (_, t, _))) => ctx |> ClosureContext.define(id, t))
  >|= AST.Raw.of_var;

let expression = (ctx: ClosureContext.t, expr) =>
  expr(ctx) >|= AST.Raw.of_expr;

let parser = (ctx: ClosureContext.t, expr) =>
  choice([variable(ctx, expr), expression(ctx, expr)]) |> M.terminated;
