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
          T.Valid(`Struct([("foo", (Valid(`Boolean), true))]))
          |> KDotAccess.Analyzer.validate_dot_access("foo"),
        )
    ),
    "dot access on type without specified property"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidDotAccess(Valid(`Integer), "foo")),
          T.Valid(`Integer) |> KDotAccess.Analyzer.validate_dot_access("foo"),
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
          |> KFunctionCall.Analyzer.validate_function_call,
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
          |> KFunctionCall.Analyzer.validate_function_call,
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
          |> KFunctionCall.Analyzer.validate_function_call,
        )
    ),
  ];
