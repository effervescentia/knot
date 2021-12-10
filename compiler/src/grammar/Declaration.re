open Kore;
open AST;

module Analyzer = Analyze.Analyzer;

let _scope_of_context = (ctx: ModuleContext.t, range: Range.t) =>
  Scope.create(
    ctx.namespace_context.namespace,
    range,
    ctx.namespace_context.report,
  );

let constant = (ctx: ModuleContext.t, f) =>
  Keyword.const
  >>= Node.Raw.get_range
  % (
    start =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, expr)) => {
          let scope = _scope_of_context(ctx, Node.Raw.get_range(expr));
          let const = Analyzer.(res_constant(res_expr(scope), expr));

          ((f(id), const), Range.join(start, Node.get_range(const)));
        }
      )
      |> M.terminated
  );

let function_ = (ctx: ModuleContext.t, f) =>
  Keyword.func
  >>= Node.Raw.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((args, res, range)) => {
              let scope = _scope_of_context(ctx, range);
              let func =
                Analyzer.(res_function(res_expr(scope), args, res, range));

              ((f(id), func), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );

let parser = (ctx: ModuleContext.t) =>
  option(of_named_export, of_main_export <$ Keyword.main)
  >>= (
    f =>
      choice([constant(ctx, f), function_(ctx, f)])
      >|= Tuple.map_fst2(of_decl)
  );
