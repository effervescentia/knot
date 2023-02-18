open Knot.Kore;
open Parse.Kore;
open AST;

let parse: Interface.Plugin.parse_t('ast) =
  (is_main, ctx) =>
    Matchers.keyword(Constants.Keyword.enum)
    >|= Node.get_range
    >>= (
      start =>
        Matchers.vertical_bar_sep(
          KTypeStatement.Parser.parse_type_variant(ctx),
        )
        // TODO: should return the range from Matchers.vertical_bar_sep or Matchers.assign
        |> Matchers.assign(KIdentifier.Parser.parse_raw(ctx))
        |> Matchers.terminated
        >|= (
          ((name, variants)) => {
            let variants' =
              variants
              |> List.map(
                   fst
                   % Tuple.map_snd2(
                       List.map(type_expr =>
                         type_expr
                         |> Node.add_type(
                              type_expr
                              |> fst
                              |> KTypeExpression.Plugin.analyze(
                                   SymbolTable.create(),
                                 ),
                            )
                       ),
                     ),
                 );
            let type_ =
              Type.Valid(
                Enumerated(
                  variants'
                  |> List.map(Tuple.map_each2(fst, List.map(Node.get_type))),
                ),
              );
            let variants_range =
              variants
              |> List.last
              |?> Node.get_range
              |?: Node.get_range(name);

            ctx.symbols
            |> SymbolTable.declare_value(~main=is_main, fst(name), type_);

            let result = Node.typed(variants', type_, variants_range);
            let range = Range.join(start, variants_range);

            Node.raw((name, result), range);
          }
        )
        |> Matchers.terminated
    );
