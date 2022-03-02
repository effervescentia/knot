open Kore;

module TypingAnalyzer = Analyze.Typing;

let suite =
  "Analyze.Typing | JSX"
  >::: [
    "class expression with invalid type"
    >: (
      () =>
        Assert.type_error(
          None,
          TypingAnalyzer.check_jsx_class_expression(Invalid(NotInferrable)),
        )
    ),
    "class expression with boolean type"
    >: (
      () =>
        Assert.type_error(
          None,
          TypingAnalyzer.check_jsx_class_expression(Valid(`Boolean)),
        )
    ),
    "class expression with non-boolean type"
    >: (
      () =>
        Assert.type_error(
          Some(InvalidJSXClassExpression(Valid(`String))),
          TypingAnalyzer.check_jsx_class_expression(Valid(`String)),
        )
    ),
    "inline expression with invalid type"
    >: (
      () =>
        Assert.type_error(
          None,
          TypingAnalyzer.check_jsx_primitive_expression(
            Invalid(NotInferrable),
          ),
        )
    ),
    "inline expression with non-function types"
    >: (
      () =>
        [
          T.Valid(`Nil),
          T.Valid(`Boolean),
          T.Valid(`Integer),
          T.Valid(`Float),
          T.Valid(`String),
          T.Valid(`Element),
        ]
        |> List.iter(type_ =>
             Assert.type_error(
               None,
               TypingAnalyzer.check_jsx_primitive_expression(type_),
             )
           )
    ),
    "inline expression with function type"
    >: (
      () =>
        Assert.type_error(
          Some(
            InvalidJSXPrimitiveExpression(
              Valid(`Function(([], Valid(`Nil)))),
            ),
          ),
          TypingAnalyzer.check_jsx_primitive_expression(
            Valid(`Function(([], Valid(`Nil)))),
          ),
        )
    ),
  ];
