open Kore;

module TypingAnalyzer = Analyze.Typing;

let suite =
  "Analyze.Typing"
  >::: [
    "dot access on type with specified property"
    >: (
      () =>
        Assert.type_error(
          None,
          T.Valid(`Struct([("foo", Valid(`Boolean))]))
          |> TypingAnalyzer.check_dot_access("foo"),
        )
    ),
    "dot access on type without specified property"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidDotAccess(Valid(`Integer), "foo")),
          T.Valid(`Integer) |> TypingAnalyzer.check_dot_access("foo"),
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
          |> TypingAnalyzer.check_function_call,
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
          |> TypingAnalyzer.check_function_call,
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
          |> TypingAnalyzer.check_function_call,
        )
    ),
  ];
