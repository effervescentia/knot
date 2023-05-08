open Kore;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, (statement, _) as node) => {
    let bind = (analyze, to_stmt, value) =>
      node
      |> Node.map(_ => value)
      |> analyze(analyze_expression, scope)
      |> Tuple.map_fst2(to_stmt);

    statement
    |> Interface.fold(
         ~variable=bind(Variable.analyze, Interface.of_variable),
         ~effect=bind(Effect.analyze, Interface.of_effect),
       );
  };
