open Kore;
open AST.Raw;

let variable = (ctx: ModuleContext.t, expr) =>
  Keyword.let_
  >> Operator.assign(Identifier.parser(ctx), expr(ctx))
  >|= of_var;

let expression = (ctx: ModuleContext.t, expr) => expr(ctx) >|= of_expr;

let parser = (ctx: ModuleContext.t, expr) =>
  choice([variable(ctx, expr), expression(ctx, expr)]) |> M.terminated;
