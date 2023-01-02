open Knot.Kore;
open AST;

let analyze_arithmetic = type_ =>
  Type.(
    switch (type_) {
    | Valid(Integer | Float)
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
  (scope, analyze_expression, (operator, expression), range) => {
    let expression' = analyze_expression(scope, expression);
    let type_ = Node.get_type(expression');

    type_
    |> Validator.validate(operator)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expression',
      switch (operator) {
      | Negative
      | Positive => analyze_arithmetic(type_)
      | Not => Valid(Boolean)
      },
    );
  };
