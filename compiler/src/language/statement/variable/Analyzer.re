open Knot.Kore;
open AST;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, ((name, expression), _)) => {
    let (expression', expression_type) =
      expression |> Node.analyzer(analyze_expression(scope));

    scope
    |> Scope.define(fst(name), expression_type)
    |> Option.iter(Scope.report_type_err(scope, Node.get_range(name)));

    ((name, expression'), Type.Valid(Nil));
  };
