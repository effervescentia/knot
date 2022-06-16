open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.enum
  >|= N2.get_range
  >>= (
    start =>
      Typing.type_variants(ctx)
      |> Operator.assign(Identifier.parser(ctx))
      |> M.terminated
      >|= (
        ((id, raw_variants)) => {
          let variants =
            raw_variants
            |> List.map(
                 fst
                 % Tuple.map_snd2(
                     List.map(type_expr =>
                       type_expr
                       |> N2.add_type(
                            type_expr
                            |> fst
                            |> Analyze.Typing.eval_type_expression,
                          )
                     ),
                   ),
               );
          let type_ =
            T.Valid(
              `Enumerated(
                variants
                |> List.map(
                     Tuple.map_each2(
                       fst % Reference.Identifier.to_string,
                       List.map(N2.get_type),
                     ),
                   ),
              ),
            );
          let range =
            Range.join(
              start,
              raw_variants |> List.last |?> N2.get_range |?: start,
            );
          let enum = N2.typed(A.of_enum(variants), type_, range);
          let export_id = f(id);

          ctx
          |> ModuleContext.declare(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          N2.untyped((export_id, enum), range);
        }
      )
      |> M.terminated
  );
