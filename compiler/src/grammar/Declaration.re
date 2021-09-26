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
        ((id, raw_expr)) => {
          let scope = _scope_of_context(ctx, Node.Raw.get_range(raw_expr));
          let expr = Analyzer.res_expr(scope, raw_expr);

          (
            (
              f(id),
              (of_const(expr), Node.get_type(expr), Node.get_range(expr)),
            ),
            Range.join(start, Node.get_range(expr)),
          );
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
            ((raw_args, raw_res, range)) => {
              let scope = _scope_of_context(ctx, range);
              let res = Analyzer.res_expr(scope, raw_res);
              let args =
                raw_args
                |> List.map(((arg, range): Raw.argument_t) =>
                     (
                       {
                         name: arg.name,
                         default:
                           arg.default
                           |> Option.map(Analyzer.res_expr(scope)),
                         type_: None,
                       },
                       /* TODO: implement */
                       Type.Valid(`Abstract(Unknown)),
                       range,
                     )
                   );

              (
                (
                  f(id),
                  (
                    (args, res) |> of_func,
                    Type.Valid(
                      `Function((
                        args
                        |> List.map((({name}, type_, _)) =>
                             (
                               name
                               |> Node.Raw.get_value
                               |> ~@Reference.Identifier.pp,
                               type_,
                             )
                           ),
                        Node.get_type(res),
                      )),
                    ),
                    range,
                  ),
                ),
                Range.join(start, range),
              );
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
