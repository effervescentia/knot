open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let _is_main: A.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;

let _create_scope = (range: Range.t, ctx: ModuleContext.t) =>
  ctx |> ModuleContext.to_scope(range);

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
          let export_id = f(id);

          ctx
          |> ModuleContext.declare(
               ~main=_is_main(export_id),
               NR.get_value(id),
               type_,
             );

          NR.create((export_id, const), range);
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
                raw_args |> SemanticAnalyzer.analyze_argument_list(scope);

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
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=_is_main(export_id),
                   NR.get_value(id),
                   type_,
                 );

              let func = N.create((args, res) |> A.of_func, type_, range);

              NR.create((export_id, func), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );

let view = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.view
  >>= NR.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((raw_props, raw_res, range)) => {
              let scope = ctx |> _create_scope(range);
              let props =
                raw_props
                |> List.map(SemanticAnalyzer.analyze_argument(scope));

              props
              |> List.iter(((arg, arg_type, arg_range)) =>
                   scope
                   |> S.define(A.(arg.name) |> NR.get_value, arg_type)
                   |> Option.iter(S.report_type_err(scope, arg_range))
                 );

              let res_scope = scope |> S.create_child(N.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_view_body(res_scope);

              let prop_types =
                props
                |> List.map(
                     Tuple.split2(
                       N.get_value
                       % A.(
                           prop =>
                             prop.name
                             |> NR.get_value
                             |> Reference.Identifier.to_string
                         ),
                       N.get_type,
                     ),
                   );
              let type_ = T.Valid(`View((prop_types, N.get_type(res))));
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=_is_main(export_id),
                   NR.get_value(id),
                   type_,
                 );

              let view = N.create((props, res) |> A.of_view, type_, range);

              NR.create((export_id, view), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );

let style_rule_set = (ctx: ModuleContext.t) =>
  choice([
    M.identifier(~prefix=Character.period)
    >|= NR.map_value(Reference.Identifier.of_string)
    >|= A.of_class_matcher,
    M.identifier(~prefix=Character.octothorpe)
    >|= NR.map_value(Reference.Identifier.of_string)
    >|= A.of_id_matcher,
  ])
  >>= (
    matcher =>
      Identifier.parser(ctx)
      >>= (
        key =>
          Symbol.colon
          >> Expression.parser(ctx)
          >|= (
            expr =>
              NR.create(
                (key, expr),
                Range.join(NR.get_range(key), N.get_range(expr)),
              )
          )
      )
      |> many
      |> M.between(Symbol.open_closure, Symbol.close_closure)
      >|= NR.map_value(Tuple.with_fst2(matcher))
  );

let style = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.style
  >>= NR.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          option([], Lambda.arguments(ctx))
          >>= (
            raw_args =>
              Glyph.lambda
              >|= NR.get_range
              >>= (
                start_range =>
                  style_rule_set(ctx)
                  |> many
                  |> M.between(Symbol.open_closure, Symbol.close_closure)
                  >|= (
                    ((raw_rule_sets, range)) => {
                      let scope = ctx |> _create_scope(range);
                      let args =
                        raw_args
                        |> SemanticAnalyzer.analyze_argument_list(scope);

                      args
                      |> List.iter(((arg, arg_type, arg_range)) =>
                           scope
                           |> S.define(
                                A.(arg.name) |> NR.get_value,
                                arg_type,
                              )
                           |> Option.iter(
                                S.report_type_err(scope, arg_range),
                              )
                         );

                      let (ids, classes) =
                        raw_rule_sets
                        |> List.map(NR.get_value % fst)
                        |> List.fold_left(
                             ((ids, classes)) =>
                               fun
                               | A.ID(id) => (ids @ [id], classes)
                               | A.Class(id) => (ids, classes @ [id]),
                             ([], []),
                           )
                        |> Tuple.map2(
                             List.map(
                               NR.get_value % Reference.Identifier.to_string,
                             ),
                           );

                      let res_rule_sets =
                        raw_rule_sets
                        |> List.map(
                             NR.map_value(
                               Tuple.map_snd2(
                                 List.map(
                                   NR.map_value(
                                     Tuple.map_snd2(
                                       SemanticAnalyzer.analyze_expression(
                                         scope,
                                       ),
                                     ),
                                   ),
                                 ),
                               ),
                             ),
                           );

                      let type_ =
                        T.Valid(
                          `Style((
                            args |> List.map(N.get_type),
                            ids,
                            classes,
                          )),
                        );
                      let export_id = f(id);

                      ctx
                      |> ModuleContext.declare(
                           ~main=_is_main(export_id),
                           NR.get_value(id),
                           type_,
                         );

                      let style =
                        N.create(
                          (args, res_rule_sets) |> A.of_style,
                          type_,
                          range,
                        );

                      NR.create(
                        (export_id, style),
                        Range.join(start, range),
                      );
                    }
                  )
              )
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
      choice([constant(ctx, f), function_(ctx, f), view(ctx, f)])
      >|= Tuple.map_fst2(A.of_decl)
  );
