open Kore;

module TypingAnalyzer = Analyze.Typing;

let __rule_name = "foo";

let suite =
  "Analyze.Typing | Style"
  >::: [
    "with both types invalid"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Invalid(NotInferrable), T.Invalid(NotInferrable))
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
    "with invalid rule type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Invalid(NotInferrable), T.Valid(`Integer))
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
    "with invalid rule expression type"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Valid(`Integer), T.Invalid(NotInferrable))
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
    "with both types valid"
    >: (
      () =>
        Assert.type_error(
          None,
          (T.Valid(`Integer), T.Valid(`Integer))
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
    "with raw string expression"
    >: (
      () =>
        Assert.type_error(
          None,
          (
            T.Valid(`Function(([T.Valid(`Integer)], T.Valid(`Nil)))),
            T.Valid(`String),
          )
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
    "throws InvalidStyleRule"
    >: (
      () =>
        Assert.type_error(
          Some(
            InvalidStyleRule(
              __rule_name,
              T.Valid(`Integer),
              T.Valid(`Boolean),
            ),
          ),
          (T.Valid(`Integer), T.Valid(`Boolean))
          |> TypingAnalyzer.check_style_rule(__rule_name),
        )
    ),
  ];
