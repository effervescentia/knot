open Knot.Kore;
open AST;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, (statements, _) as node) => {
    let range = Node.get_range(node);
    let closure_scope = scope |> Scope.create_child(range);
    let (statements', statement_types) =
      statements
      |> List.map(
           Node.analyzer(
             KStatement.Analyzer.analyze(analyze_expression, closure_scope),
           ),
         )
      |> List.split;

    (statements', List.last(statement_types) |?: Type.Valid(Nil));
  };
