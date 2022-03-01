open Kore;

module TypingAnalyzer = Analyze.Typing;
module TE = ASTV2.TypeExpression;
module U = Util.RawUtil;

let suite =
  "Analyze.Typing | Type Expression"
  >::: [
    "nil"
    >: (
      () =>
        Assert.typeV2(Valid(`Nil), TypingAnalyzer.eval_type_expression(Nil))
    ),
    "boolean"
    >: (
      () =>
        Assert.typeV2(
          Valid(`Boolean),
          TypingAnalyzer.eval_type_expression(Boolean),
        )
    ),
    "integer"
    >: (
      () =>
        Assert.typeV2(
          Valid(`Integer),
          TypingAnalyzer.eval_type_expression(Integer),
        )
    ),
    "float"
    >: (
      () =>
        Assert.typeV2(
          Valid(`Float),
          TypingAnalyzer.eval_type_expression(Float),
        )
    ),
    "string"
    >: (
      () =>
        Assert.typeV2(
          Valid(`String),
          TypingAnalyzer.eval_type_expression(String),
        )
    ),
    "element"
    >: (
      () =>
        Assert.typeV2(
          Valid(`Element),
          TypingAnalyzer.eval_type_expression(Element),
        )
    ),
    "grouped type"
    >: (
      () =>
        Assert.typeV2(
          Valid(`String),
          TE.String
          |> U.as_raw_node
          |> TE.of_group
          |> U.as_raw_node
          |> TE.of_group
          |> U.as_raw_node
          |> TE.of_group
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "list type"
    >: (
      () =>
        Assert.typeV2(
          Valid(`List(Valid(`Boolean))),
          TE.Boolean
          |> U.as_raw_node
          |> TE.of_list
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "struct type"
    >: (
      () =>
        Assert.typeV2(
          Valid(
            `Struct([("foo", Valid(`Boolean)), ("bar", Valid(`String))]),
          ),
          [
            (U.as_raw_node("foo"), U.as_raw_node(TE.Boolean)),
            (U.as_raw_node("bar"), U.as_raw_node(TE.String)),
          ]
          |> TE.of_struct
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
    "function type"
    >: (
      () =>
        Assert.typeV2(
          Valid(
            `Function((
              [Valid(`Boolean), Valid(`String)],
              Valid(`Element),
            )),
          ),
          (
            [U.as_raw_node(TE.Boolean), U.as_raw_node(TE.String)],
            U.as_raw_node(TE.Element),
          )
          |> TE.of_function
          |> TypingAnalyzer.eval_type_expression,
        )
    ),
  ];
