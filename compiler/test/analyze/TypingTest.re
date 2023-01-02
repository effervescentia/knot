open Kore;

module T = AST.Type;

let suite =
  "Analyze.Typing"
  >::: [
    "dot access on type with specified property"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Valid(`Object([("foo", (Valid(`Boolean), true))]))
          |> KDotAccess.Validator.validate("foo"),
        )
    ),
    "dot access on type without specified property"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidDotAccess(Valid(`Integer), "foo")),
          T.Valid(`Integer) |> KDotAccess.Validator.validate("foo"),
        )
    ),
    "function call on type with matching arguments"
    >: (
      () =>
        Assert.type_error(
          None,
          (
            T.Valid(
              `Function(([Valid(`Boolean), Valid(`String)], Valid(`Nil))),
            ),
            [T.Valid(`Boolean), T.Valid(`String)],
          )
          |> KFunctionCall.Validator.validate,
        )
    ),
    "function call on type with invalid argument"
    >: (
      () =>
        Assert.type_error(
          None,
          (
            T.Valid(
              `Function(([Valid(`Boolean), Valid(`String)], Valid(`Nil))),
            ),
            [T.Valid(`Boolean), T.Invalid(NotInferrable)],
          )
          |> KFunctionCall.Validator.validate,
        )
    ),
    "function call on type without matching arguments"
    >: (
      () =>
        Assert.type_error(
          Some(
            InvalidFunctionCall(
              Valid(
                `Function((
                  [Valid(`Boolean), Valid(`String)],
                  Valid(`Nil),
                )),
              ),
              [T.Valid(`Boolean)],
            ),
          ),
          (
            T.Valid(
              `Function(([Valid(`Boolean), Valid(`String)], Valid(`Nil))),
            ),
            [T.Valid(`Boolean)],
          )
          |> KFunctionCall.Validator.validate,
        )
    ),
  ];
