open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let _create_scope = (range: Range.t, ctx: ModuleContext.t) =>
  ScopeV2.create(
    ctx.namespace_context.namespace,
    ctx.namespace_context.report,
    range,
  );

let constant = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.const
  >|= NR.get_range
  >>= (
    start =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, (_, _, expr_range) as raw_expr)) => {
          let scope = ctx |> _create_scope(expr_range);
          let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
          let type_ = N.get_type(expr);
          let const = expr |> N.wrap(A.of_const);
          let range = Range.join(start, expr_range);

          scope
          |> S.define(NR.get_value(id), type_)
          |> Option.iter(S.report_type_err(scope, NR.get_range(id)));

          NR.create((f(id), const), range);
        }
      )
      |> M.terminated
  );

let function_ = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.func
  >>= NR.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((raw_args, raw_res, range)) => {
              let scope = ctx |> _create_scope(range);
              let args =
                raw_args
                |> List.map(SemanticAnalyzer.analyze_argument(scope));

              args
              |> List.iter(((arg, arg_type, arg_range)) =>
                   scope
                   |> S.define(A.(arg.name) |> NR.get_value, arg_type)
                   |> Option.iter(S.report_type_err(scope, arg_range))
                 );

              let res_scope = scope |> S.create_child(N.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_expression(res_scope);

              let type_ =
                T.Valid(
                  `Function((
                    args |> List.map(N.get_type),
                    N.get_type(res),
                  )),
                );
              let func = N.create((args, res) |> A.of_func, type_, range);

              NR.create((f(id), func), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );

let parser = (ctx: ModuleContext.t) =>
  A.of_main_export
  <$ Keyword.main
  |> option(A.of_named_export)
  >>= (
    f =>
      choice([constant(ctx, f), function_(ctx, f)])
      >|= Tuple.map_fst2(A.of_decl)
  );
