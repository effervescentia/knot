open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let style_rule_set = (ctx: ParseContext.t) =>
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
        key =>
          Symbol.colon
          >> Expression.parser(ctx)
          >|= (expr => N.untyped((key, expr), N.join_ranges(key, expr)))
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
                start_range =>
                  style_rule_set(ctx)
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

                      let res_rule_sets =
                        raw_rule_sets
                        |> fst
                        |> List.map(
                             N.map(
                               Tuple.map_snd2(
                                 List.map(
                                   N.map(
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
                  )
              )
          )
      )
      |> M.terminated
  );
