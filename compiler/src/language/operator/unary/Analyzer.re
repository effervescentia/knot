open Knot.Kore;

let validate_unary_operation: (AST.unary_t, Type.t) => option(Type.error_t) =
  op =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(valid_type) as type_ =>
      switch (op, valid_type) {
      | (Negative | Positive, `Integer | `Float) => None
      | (Not, `Boolean) => None

      | _ => Some(InvalidUnaryOperation(op, type_))
      };

let analyze_unary_operation:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    (AST.unary_t, AST.Raw.expression_t),
    Range.t
  ) =>
  (AST.expression_t, Type.t) =
  (scope, analyze_expression, (op, expr), range) => {
    let expr' = analyze_expression(scope, expr);
    let type_ = Node.get_type(expr');

    type_
    |> validate_unary_operation(op)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expr',
      switch (op) {
      | Negative
      | Positive =>
        switch (type_) {
        | Valid(`Integer | `Float)
        /* forward invalid types */
        | Invalid(_) => type_

        | _ => Invalid(NotInferrable)
        }

      | Not => Valid(`Boolean)
      },
    );
  };
