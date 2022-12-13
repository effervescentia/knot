open Knot.Kore;
open AST;

let analyze_arithmetic = type_ =>
  Type.(
    switch (type_) {
    | Valid(`Integer | `Float)
    /* forward invalid types */
    | Invalid(_) => type_

    | _ => Invalid(NotInferrable)
    }
  );

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Operator.Unary.t, Raw.expression_t),
    Range.t
  ) =>
  (Result.expression_t, Type.t) =
  (scope, analyze_expression, (op, expr), range) => {
    let expr' = analyze_expression(scope, expr);
    let type_ = Node.get_type(expr');

    type_
    |> Validator.validate(op)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expr',
      switch (op) {
      | Negative
      | Positive => analyze_arithmetic(type_)
      | Not => Valid(`Boolean)
      },
    );
  };
