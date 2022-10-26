open Kore;

let suite =
  "Analyze.Typing | Binary"
  >::: [
    "with invalid lhs type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Invalid(NotInferrable), T.Valid(`Boolean))
          |> KBinaryOperator.Analyzer.validate_binary_operation(LogicalAnd),
        )
    ),
    "with invalid rhs type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Valid(`Boolean), T.Invalid(NotInferrable))
          |> KBinaryOperator.Analyzer.validate_binary_operation(LogicalAnd),
        )
    ),
    "'and' (&&) and 'or' (||) operations with boolean types"
    >: (
      () =>
        [A.LogicalAnd, A.LogicalOr]
        |> List.iter(op =>
             Assert.type_error(
               None,
               (T.Valid(`Boolean), T.Valid(`Boolean))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
    "'and' (&&) and 'or' (||) operations with non-boolean types"
    >: (
      () =>
        [A.LogicalAnd, A.LogicalOr]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(
                   op,
                   Valid(`String),
                   Valid(`Integer),
                 ),
               ),
               (T.Valid(`String), T.Valid(`Integer))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
    "arithmetic operations (+, -, *, /) with combination of integer and float types"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply, A.Divide]
        |> List.iter(op =>
             [
               (T.Valid(`Integer), T.Valid(`Integer)),
               (T.Valid(`Integer), T.Valid(`Float)),
               (T.Valid(`Float), T.Valid(`Integer)),
               (T.Valid(`Float), T.Valid(`Float)),
             ]
             |> List.iter(types =>
                  Assert.type_error(
                    None,
                    types
                    |> KBinaryOperator.Analyzer.validate_binary_operation(op),
                  )
                )
           )
    ),
    "arithmetic operations (+, -, *, /) with non-numeric types"
    >: (
      () =>
        [A.Add, A.Subtract, A.Multiply, A.Divide]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(
                   op,
                   Valid(`String),
                   Valid(`Boolean),
                 ),
               ),
               (T.Valid(`String), T.Valid(`Boolean))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
    "comparative operations (<, <=, >, >=) with combination of integer and float types"
    >: (
      () =>
        [A.LessThan, A.LessOrEqual, A.GreaterThan, A.GreaterOrEqual]
        |> List.iter(op =>
             [
               (T.Valid(`Integer), T.Valid(`Integer)),
               (T.Valid(`Integer), T.Valid(`Float)),
               (T.Valid(`Float), T.Valid(`Integer)),
               (T.Valid(`Float), T.Valid(`Float)),
             ]
             |> List.iter(types =>
                  Assert.type_error(
                    None,
                    types
                    |> KBinaryOperator.Analyzer.validate_binary_operation(op),
                  )
                )
           )
    ),
    "comparative operations (<, <=, >, >=) with non-numeric types"
    >: (
      () =>
        [A.LessThan, A.LessOrEqual, A.GreaterThan, A.GreaterOrEqual]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(
                   op,
                   Valid(`String),
                   Valid(`Boolean),
                 ),
               ),
               (T.Valid(`String), T.Valid(`Boolean))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
    "'equal' (==) and 'unequal' (!=) operations with the equal types"
    >: (
      () =>
        [A.Equal, A.Unequal]
        |> List.iter(op =>
             Assert.type_error(
               None,
               (T.Valid(`String), T.Valid(`String))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
    "'and' (&&) and 'or' (||) operations with non-equal types"
    >: (
      () =>
        [A.Equal, A.Unequal]
        |> List.iter(op =>
             Assert.type_error(
               Some(
                 InvalidBinaryOperation(
                   op,
                   Valid(`String),
                   Valid(`Integer),
                 ),
               ),
               (T.Valid(`String), T.Valid(`Integer))
               |> KBinaryOperator.Analyzer.validate_binary_operation(op),
             )
           )
    ),
  ];
