open Kore;

module TypingAnalyzer = Analyze.Typing;

let suite =
  "Analyze.Typing | Unary Operation"
  >::: [
    "with invalid type"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Invalid(NotInferrable)
          |> TypingAnalyzer.check_unary_operation(Not),
        )
    ),
    "'not' (!) operation with boolean type"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Valid(`Boolean) |> TypingAnalyzer.check_unary_operation(Not),
        )
    ),
    "'not' (!) operation with non-boolean type"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidUnaryOperation(Not, Valid(`String))),
          T.Valid(`String) |> TypingAnalyzer.check_unary_operation(Not),
        )
    ),
    "'positive' (+) and 'negative' (-) operations with integer type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.type_error(
               None,
               T.Valid(`Integer) |> TypingAnalyzer.check_unary_operation(op),
             )
           )
    ),
    "'positive' (+) and 'negative' (-) operations with float type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.type_error(
               None,
               T.Valid(`Float) |> TypingAnalyzer.check_unary_operation(op),
             )
           )
    ),
    "'positive' (+) and 'negative' (-) operations with non-numeric type"
    >: (
      () =>
        [A.Positive, A.Negative]
        |> List.iter(op =>
             Assert.type_error(
               Some(InvalidUnaryOperation(op, Valid(`String))),
               T.Valid(`String) |> TypingAnalyzer.check_unary_operation(op),
             )
           )
    ),
  ];
