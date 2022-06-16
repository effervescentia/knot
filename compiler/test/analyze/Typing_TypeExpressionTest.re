open Kore;

module TypingAnalyzer = Analyze.Typing;
module TE = AST.TypeExpression;
module U = Util.RawUtil;

let suite =
  "Analyze.Typing | Type Expression"
  >::: [
    "nil"
    >: (
      () =>
        Assert.type_(Valid(`Nil), TypingAnalyzer.eval_type_expression(Nil))
    ),
    "boolean"
    >: (
      () =>
        Assert.type_(
          Valid(`Boolean),
          TypingAnalyzer.eval_type_expression(Boolean),
        )
    ),
    "integer"
    >: (
      () =>
        Assert.type_(
          Valid(`Integer),
          TypingAnalyzer.eval_type_expression(Integer),
        )
    ),
    "float"
    >: (
      () =>
        Assert.type_(
          Valid(`Float),
          TypingAnalyzer.eval_type_expression(Float),
        )
    ),
    "string"
    >: (
      () =>
        Assert.type_(
          Valid(`String),
          TypingAnalyzer.eval_type_expression(String),
        )
    ),
    "element"
    >: (
      () =>
        Assert.type_(
          Valid(`Element),
          TypingAnalyzer.eval_type_expression(Element),
        )
    ),
    "grouped type"
    >: (
      () =>
        Assert.type_(
          Valid(`String),
          TE.String
          |> U.as_untyped
          |> TE.of_group
          |> U.as_untyped
          |> TE.of_group
          |> U.as_untyped
          |> TE.of_group
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "list type"
    >: (
      () =>
        Assert.type_(
          Valid(`List(Valid(`Boolean))),
          TE.Boolean
          |> U.as_untyped
          |> TE.of_list
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "struct type"
    >: (
      () =>
        Assert.type_(
          Valid(
            `Struct([("foo", Valid(`Boolean)), ("bar", Valid(`String))]),
          ),
          [
            (U.as_untyped("foo"), U.as_untyped(TE.Boolean)),
            (U.as_untyped("bar"), U.as_untyped(TE.String)),
          ]
          |> TE.of_struct
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "function type"
    >: (
      () =>
        Assert.type_(
          Valid(
            `Function((
              [Valid(`Boolean), Valid(`String)],
              Valid(`Element),
            )),
          ),
          (
            [U.as_untyped(TE.Boolean), U.as_untyped(TE.String)],
            U.as_untyped(TE.Element),
          )
          |> TE.of_function
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
  ];
