open Kore;
open AST;

module Analyzer = Analyze.Analyzer;
module Scope = Analyze.Scope;

let _scope_of_context = (ctx: ModuleContext.t) =>
  Scope.create(ctx.namespace_context.namespace, ctx.namespace_context.report);

let constant = (ctx: ModuleContext.t, f) =>
  Keyword.const
  >>= Node.Raw.cursor
  % (
    start =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, raw_expr)) => {
          let scope = _scope_of_context(ctx);
          let expr = Analyzer.res_expr(scope, raw_expr);

          (
            (
              f(id),
              (of_const(expr), Node.type_(expr), Node.cursor(expr)),
            ),
            Cursor.join(start, Node.cursor(expr)),
          );
        }
      )
      |> M.terminated
  );

let function_ = (ctx: ModuleContext.t, f) =>
  Keyword.func
  >>= Node.Raw.cursor
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((raw_args, raw_res, cursor)) => {
              let scope = _scope_of_context(ctx);
              let res = Analyzer.res_expr(scope, raw_res);
              let args =
                raw_args
                |> List.map(((arg, cursor): Raw.argument_t) =>
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
                       cursor,
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
                               |> Node.Raw.value
                               |> Reference.Identifier.to_string,
                               type_,
                             )
                           ),
                        Node.type_(res),
                      )),
                    ),
                    cursor,
                  ),
                ),
                Cursor.join(start, cursor),
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
