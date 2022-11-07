open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let validate_binary_operation:
  (AST.Operator.Binary.t, (Type.t, Type.t)) => option(Type.error_t) =
  op =>
    fun
    /* assume they have been reported already and ignore */
    | (Invalid(_), _)
    | (_, Invalid(_)) => None

    | (Valid(valid_lhs) as lhs, Valid(valid_rhs) as rhs) =>
      switch (op, valid_lhs, valid_rhs) {
      | (LogicalAnd | LogicalOr, `Boolean, `Boolean) => None

      | (
          LessOrEqual | LessThan | GreaterOrEqual | GreaterThan | Add | Subtract |
          Divide |
          Multiply |
          Exponent,
          `Integer | `Float,
          `Integer | `Float,
        ) =>
        None

      | (Equal | Unequal, _, _) when valid_lhs == valid_rhs => None

      | _ => Some(InvalidBinaryOperation(op, lhs, rhs))
      };

let analyze_binary_operation:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    (AST.Operator.Binary.t, AST.Raw.expression_t, AST.Raw.expression_t),
    Range.t
  ) =>
  (AST.Result.expression_t, AST.Result.expression_t, Type.t) =
  (scope, analyze_expression, (op, lhs, rhs), range) => {
    let lhs' = analyze_expression(scope, lhs);
    let rhs' = analyze_expression(scope, rhs);
    let type_lhs = Node.get_type(lhs');
    let type_rhs = Node.get_type(rhs');

    (type_lhs, type_rhs)
    |> validate_binary_operation(op)
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
      | Unequal => Valid(`Boolean)

      | Divide
      | Exponent => Valid(`Float)

      | Add
      | Subtract
      | Multiply =>
        switch (type_lhs, type_rhs) {
        | (Valid(`Integer), Valid(`Integer)) => Valid(`Integer)

        | (_, Valid(`Float))
        | (Valid(`Float), _) => Valid(`Float)

        /* forward invalid types */
        | (Invalid(_), _) => type_lhs
        | (_, Invalid(_)) => type_rhs

        | _ => Invalid(NotInferrable)
        }
      },
    );
  };
