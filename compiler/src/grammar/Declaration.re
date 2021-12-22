open Kore;
open AST;

module Analyzer = Analyze.Analyzer;

let constant = (ctx: ModuleContext.t, f) =>
  Keyword.const
  >>= Node.Raw.get_range
  % (
    start =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, expr)) => {
          let scope = ctx.scope.create(Node.Raw.get_range(expr));

          let const =
            switch (
              scope
              |> Scope.peek(() => Analyzer.res_constant(Analyze, scope, expr))
            ) {
            | Some(const) => const
            | None => Analyzer.res_constant(Resolve, scope, expr)
            };

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
              let scope = ctx.scope.create(range);
              let func = Analyzer.res_function(scope, args, res, range);

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
