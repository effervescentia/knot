open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Operator.Binary.t, Raw.expression_t, Raw.expression_t),
    Range.t
  ) =>
  (Result.expression_t, Result.expression_t, Type.t) =
  (scope, analyze_expression, (op, lhs, rhs), range) => {
    let lhs' = analyze_expression(scope, lhs);
    let rhs' = analyze_expression(scope, rhs);
    let type_lhs = Node.get_type(lhs');
    let type_rhs = Node.get_type(rhs');

    (type_lhs, type_rhs)
    |> Validator.validate(op)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      lhs',
      rhs',
      switch (op) {
      | LogicalAnd
      | LogicalOr
      | LessOrEqual
      | LessThan
      | GreaterOrEqual
      | GreaterThan
      | Equal
      | Unequal => Valid(Boolean)

      | Divide
      | Exponent => Valid(Float)

      | Add
      | Subtract
      | Multiply =>
        switch (type_lhs, type_rhs) {
        | (Valid(Integer), Valid(Integer)) => Valid(Integer)

        | (_, Valid(Float))
        | (Valid(Float), _) => Valid(Float)

        /* forward invalid types */
        | (Invalid(_), _) => type_lhs
        | (_, Invalid(_)) => type_rhs

        | _ => Invalid(NotInferrable)
        }
      },
    );
  };
