open Knot.Kore;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, (expression, _)) =>
    expression |> Node.analyzer(analyze_expression(scope));
