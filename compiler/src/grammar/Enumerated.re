open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let variant = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Typing.expression_parser
      |> M.comma_sep
      |> M.between(Symbol.open_group, Symbol.close_group)
      |> option(([], NR.get_range(id)))
      >|= NR.map_value(Tuple.with_fst2(id))
  );

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.enum
  >|= NR.get_range
  >>= (
    start =>
      optional(Symbol.vertical_bar)
      >> (variant(ctx) |> sep_by(Symbol.vertical_bar))
      |> Operator.assign(Identifier.parser(ctx))
      >|= (
        ((id, raw_variants)) => {
          let variants =
            raw_variants
            |> List.map(
                 NR.get_value
                 % Tuple.map_snd2(
                     List.map(type_expr =>
                       type_expr
                       |> N.of_raw(
                            type_expr
                            |> NR.get_value
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
                       NR.get_value % Reference.Identifier.to_string,
                       List.map(N.get_type),
                     ),
                   ),
              ),
            );
          let range =
            Range.join(
              start,
              raw_variants |> List.last |?> NR.get_range |?: start,
            );
          let enum = N.create(A.of_enum(variants), type_, range);
          let export_id = f(id);

          ctx
          |> ModuleContext.declare(
               ~main=Util.is_main(export_id),
               NR.get_value(id),
               type_,
             );

          NR.create((export_id, enum), range);
        }
      )
      |> M.terminated
  );
