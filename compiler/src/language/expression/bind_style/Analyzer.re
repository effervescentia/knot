open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.expression_t, Raw.expression_t),
    Range.t
  ) =>
  (Result.expression_t, Result.expression_t) =
  (scope, analyze_expression, (view, style), range) => {
    let view' = analyze_expression(scope, view);
    let style' = analyze_expression(scope, style);
    let view_type = Node.get_type(view');
    let style_type = Node.get_type(style');

    (view_type, style_type)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (view', style');
  };
