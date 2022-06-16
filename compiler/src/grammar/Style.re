open Kore;

module SemanticAnalyzer = Analyze.Semantic;

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

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
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
                      let scope = ctx |> Util.create_scope(range);
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
                           ~main=Util.is_main(export_id),
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
