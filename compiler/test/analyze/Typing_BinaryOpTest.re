open Kore;

module A = AST.Result;
module OB = AST.Operator.Binary;
module T = AST.Type;

let suite =
  "Analyze.Typing | Binary"
  >::: [
    "with invalid lhs type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Invalid(NotInferrable), T.Valid(Boolean))
          |> KBinaryOperator.Validator.validate(LogicalAnd),
        )
    ),
    "with invalid rhs type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Valid(Boolean), T.Invalid(NotInferrable))
          |> KBinaryOperator.Validator.validate(LogicalAnd),
        )
    ),
    "'and' (&&) and 'or' (||) operations with boolean types"
    >: (
      () =>
        [OB.LogicalAnd, OB.LogicalOr]
        |> List.iter(op =>
             Assert.type_error(
               None,
               (T.Valid(Boolean), T.Valid(Boolean))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
    "'and' (&&) and 'or' (||) operations with non-boolean types"
    >: (
      () =>
        [OB.LogicalAnd, OB.LogicalOr]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(op, Valid(String), Valid(Integer)),
               ),
               (T.Valid(String), T.Valid(Integer))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
    "arithmetic operations (+, -, *, /) with combination of integer and float types"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply, OB.Divide]
        |> List.iter(op =>
             [
               (T.Valid(Integer), T.Valid(Integer)),
               (T.Valid(Integer), T.Valid(Float)),
               (T.Valid(Float), T.Valid(Integer)),
               (T.Valid(Float), T.Valid(Float)),
             ]
             |> List.iter(types =>
                  Assert.type_error(
                    None,
                    types |> KBinaryOperator.Validator.validate(op),
                  )
                )
           )
    ),
    "arithmetic operations (+, -, *, /) with non-numeric types"
    >: (
      () =>
        [OB.Add, OB.Subtract, OB.Multiply, OB.Divide]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(op, Valid(String), Valid(Boolean)),
               ),
               (T.Valid(String), T.Valid(Boolean))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
    "comparative operations (<, <=, >, >=) with combination of integer and float types"
    >: (
      () =>
        [OB.LessThan, OB.LessOrEqual, OB.GreaterThan, OB.GreaterOrEqual]
        |> List.iter(op =>
             [
               (T.Valid(Integer), T.Valid(Integer)),
               (T.Valid(Integer), T.Valid(Float)),
               (T.Valid(Float), T.Valid(Integer)),
               (T.Valid(Float), T.Valid(Float)),
             ]
             |> List.iter(types =>
                  Assert.type_error(
                    None,
                    types |> KBinaryOperator.Validator.validate(op),
                  )
                )
           )
    ),
    "comparative operations (<, <=, >, >=) with non-numeric types"
    >: (
      () =>
        [OB.LessThan, OB.LessOrEqual, OB.GreaterThan, OB.GreaterOrEqual]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(op, Valid(String), Valid(Boolean)),
               ),
               (T.Valid(String), T.Valid(Boolean))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
    "'equal' (==) and 'unequal' (!=) operations with the equal types"
    >: (
      () =>
        [OB.Equal, OB.Unequal]
        |> List.iter(op =>
             Assert.type_error(
               None,
               (T.Valid(String), T.Valid(String))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
    "'and' (&&) and 'or' (||) operations with non-equal types"
    >: (
      () =>
        [OB.Equal, OB.Unequal]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(op, Valid(String), Valid(Integer)),
               ),
               (T.Valid(String), T.Valid(Integer))
               |> KBinaryOperator.Validator.validate(op),
             )
           )
    ),
  ];
