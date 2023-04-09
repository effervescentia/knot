open Kore;

module OU = AST.Operator.Unary;
module T = AST.Type;

let suite =
  "Analyze.Typing | Unary Operation"
  >::: [
    "with invalid type"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Invalid(NotInferrable) |> KUnaryOperator.Validator.validate(Not),
        )
    ),
    "'not' (!) operation with boolean type"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Valid(Boolean) |> KUnaryOperator.Validator.validate(Not),
        )
    ),
    "'not' (!) operation with non-boolean type"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidUnaryOperation(Not, Valid(String))),
          T.Valid(String) |> KUnaryOperator.Validator.validate(Not),
        )
    ),
    "'positive' (+) and 'negative' (-) operations with integer type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.type_error(
               None,
               T.Valid(Integer) |> KUnaryOperator.Validator.validate(op),
             )
           )
    ),
    "'positive' (+) and 'negative' (-) operations with float type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.type_error(
               None,
               T.Valid(Float) |> KUnaryOperator.Validator.validate(op),
             )
           )
    ),
    "'positive' (+) and 'negative' (-) operations with non-numeric type"
    >: (
      () =>
        [OU.Positive, OU.Negative]
        |> List.iter(op =>
             Assert.type_error(
               Some(InvalidUnaryOperation(op, Valid(String))),
               T.Valid(String) |> KUnaryOperator.Validator.validate(op),
             )
           )
    ),
  ];
