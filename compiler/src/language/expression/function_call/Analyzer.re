open Knot.Kore;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, ((function_, arguments), _) as node) => {
    let range = Node.get_range(node);
    let analyze_expression = Node.analyzer(analyze_expression(scope));
    let (function_', function_type) = analyze_expression(function_);
    let (arguments', argument_types) =
      arguments |> List.map(analyze_expression) |> List.split;

    (function_type, argument_types)
    |> Validator.validate
    |> Option.iter(AST.Scope.report_type_err(scope, range));

    let result_type =
      switch (function_type) {
      | Valid(Function(_, result)) => result
      | _ => Invalid(NotInferrable)
      };

    ((function_', arguments'), result_type);
  };
