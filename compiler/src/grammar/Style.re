open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let style_rule_set = (ctx: ParseContext.t, rule_scope: Scope.t) =>
  choice([
    M.identifier(~prefix=Character.period)
    >|= N.map(String.drop_prefix("."))
    >|= A.of_class_matcher,
    M.identifier(~prefix=Character.octothorpe)
    >|= N.map(String.drop_prefix("#"))
    >|= A.of_id_matcher,
  ])
  >>= (
    matcher =>
      Identifier.parser(ctx)
      >>= (
        rule =>
          Symbol.colon
          >> Expression.parser(ctx)
          >|= (
            expr => {
              let found_type = rule_scope |> Scope.lookup(fst(rule));

              switch (found_type) {
              /* only allow single-argument void functions to be treated as rules */
              | Some(Valid(`Function([_], Valid(`Nil)))) => ()

              | _ =>
                ctx
                |> ParseContext.report(
                     TypeError(UnknownStyleRule(fst(rule))),
                     N.get_range(rule),
                   )
              };

              N.untyped(
                (
                  N.typed(
                    fst(rule),
                    found_type |?: T.Invalid(NotInferrable),
                    N.get_range(rule),
                  ),
                  expr,
                ),
                N.join_ranges(rule, expr),
              );
            }
          )
          |> M.terminated
      )
      |> many
      |> M.between(Symbol.open_closure, Symbol.close_closure)
      >|= N.map(Tuple.with_fst2(matcher))
  );

let parser = (ctx: ParseContext.t, f): declaration_parser_t =>
  Keyword.style
  >>= N.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          option([], Lambda.arguments(ctx))
          >>= (
            raw_args =>
              Glyph.lambda
              >|= N.get_range
              >>= (
                start_range => {
                  let rule_scope = Scope.create(ctx, Range.zero);

                  Scope.inject_plugin_types(
                    ~prefix="",
                    StyleRule,
                    rule_scope,
                  );

                  style_rule_set(ctx, rule_scope)
                  |> many
                  |> M.between(Symbol.open_closure, Symbol.close_closure)
                  >|= (
                    raw_rule_sets => {
                      let range = N.get_range(raw_rule_sets);
                      let scope = ctx |> Scope.of_parse_context(range);

                      Scope.inject_plugin_types(StyleExpression, scope);

                      let args =
                        raw_args
                        |> SemanticAnalyzer.analyze_argument_list(scope);

                      args
                      |> List.iter(arg =>
                           scope
                           |> S.define(
                                A.(fst(arg).name) |> fst,
                                N.get_type(arg),
                              )
                           |> Option.iter(
                                S.report_type_err(scope, N.get_range(arg)),
                              )
                         );

                      let (ids, classes) =
                        raw_rule_sets
                        |> fst
                        |> List.map(fst % fst)
                        |> List.fold_left(
                             ((ids, classes)) =>
                               fun
                               | A.MatchID((id, _)) => (ids @ [id], classes)
                               | A.MatchClass((id, _)) => (
                                   ids,
                                   classes @ [id],
                                 ),
                             ([], []),
                           );

                      let analyze_expr =
                        SemanticAnalyzer.analyze_expression(scope);
                      let res_rule_sets =
                        raw_rule_sets
                        |> fst
                        |> List.map(
                             N.map(
                               Tuple.map_snd2(
                                 List.map(
                                   N.map(((rule, raw_expr)) => {
                                     let expr = analyze_expr(raw_expr);

                                     switch (
                                       N.get_type(rule),
                                       N.get_type(expr),
                                     ) {
                                     | (
                                         T.Valid(
                                           `Function(
                                             [arg_type],
                                             Valid(`Nil),
                                           ),
                                         ),
                                         expr_type,
                                       )
                                         when arg_type != expr_type =>
                                       ctx
                                       |> ParseContext.report(
                                            TypeError(
                                              TypeMismatch(
                                                arg_type,
                                                expr_type,
                                              ),
                                            ),
                                            N.get_range(expr),
                                          )
                                     | _ => ()
                                     };

                                     (rule, expr);
                                   }),
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

                      ctx.symbols
                      |> SymbolTable.declare_value(
                           ~main=Util.is_main(export_id),
                           fst(id),
                           type_,
                         );

                      let style =
                        N.typed(
                          (args, res_rule_sets) |> A.of_style,
                          type_,
                          range,
                        );

                      N.untyped(
                        (export_id, style),
                        Range.join(start, range),
                      );
                    }
                  );
                }
              )
          )
      )
  );
