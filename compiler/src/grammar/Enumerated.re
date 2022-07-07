open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ParseContext.t, f): declaration_parser_t =>
  Keyword.enum
  >|= N.get_range
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
                       |> N.add_type(
                            type_expr
                            |> fst
                            |> Analyze.Typing.eval_type_expression(
                                 SymbolTable.create(),
                               ),
                          )
                     ),
                   ),
               );
          let type_ =
            T.Valid(
              `Enumerated(
                variants
                |> List.map(Tuple.map_each2(fst, List.map(N.get_type))),
              ),
            );
          let range =
            Range.join(
              start,
              raw_variants |> List.last |?> N.get_range |?: start,
            );
          let enum = N.typed(A.of_enum(variants), type_, range);
          let export_id = f(id);

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          N.untyped((export_id, enum), range);
        }
      )
      |> M.terminated
  );
