open Knot.Kore;
open AST;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, ((operator, lhs, rhs), _) as node) => {
    let range = Node.get_range(node);
    let (lhs', lhs_type) = lhs |> Node.analyzer(analyze_expression(scope));
    let (rhs', rhs_type) = rhs |> Node.analyzer(analyze_expression(scope));

    (lhs_type, rhs_type)
    |> Validator.validate(operator)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      (operator, lhs', rhs'),
      switch (operator) {
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
        switch (lhs_type, rhs_type) {
        | (Valid(Integer), Valid(Integer)) => Valid(Integer)

        | (_, Valid(Float))
        | (Valid(Float), _) => Valid(Float)

        /* forward invalid types */
        | (Invalid(_), _) => lhs_type
        | (_, Invalid(_)) => rhs_type

        | _ => Invalid(NotInferrable)
        }
      },
    );
  };
