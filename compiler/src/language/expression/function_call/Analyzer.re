open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.expression_t, list(Raw.expression_t)),
    Range.t
  ) =>
  (Result.expression_t, list(Result.expression_t), Type.t) =
  (scope, analyze_expression, (expr, args), range) => {
    let expr' = analyze_expression(scope, expr);
    let args' = args |> List.map(analyze_expression(scope));
    let type_expr = Node.get_type(expr');
    let type_args = args' |> List.map(Node.get_type);

    (type_expr, type_args)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expr',
      args',
      switch (type_expr) {
      | Valid(`Function(_, result)) => result
      | _ => Invalid(NotInferrable)
      },
    );
  };
