open Knot.Kore;
open AST;

let validate_style_binding: ((Type.t, Type.t)) => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | (Invalid(_), _)
  | (_, Invalid(_)) => None

  | (Valid(`View(_)), Valid(`Style)) => None

  | (view, style) => Some(InvalidStyleBinding(view, style));

let analyze_bind_style:
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
    |> validate_style_binding
    |> Option.iter(Scope.report_type_err(scope, range));

    (view', style');
  };
