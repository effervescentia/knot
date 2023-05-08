open Kore;
open AST;

let analyze_view_body:
  Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr) =
  (scope, body) => {
    let (body', body_type) =
      body |> Node.analyzer(Expression.analyze(scope));

    body_type
    |> Validator.validate_jsx_primitive_expression
    |> Option.iter(body' |> Node.get_range |> Scope.report_type_err(scope));

    (body', body_type);
  };
