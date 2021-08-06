open Kore;
open AST.Raw;

let variable = (ctx: ModuleContext.t, expr) =>
  Keyword.let_
  >>= Node.Raw.range
  % (
    start =>
      Operator.assign(Identifier.parser(ctx), expr(ctx))
      >|= (x => (of_var(x), Range.join(start, x |> snd |> Node.Raw.range)))
  );

let expression = (ctx: ModuleContext.t, expr) =>
  expr(ctx) >|= (x => (of_expr(x), Node.Raw.range(x)));

let parser = (ctx: ModuleContext.t, expr) =>
  choice([variable(ctx, expr), expression(ctx, expr)]) |> M.terminated;
