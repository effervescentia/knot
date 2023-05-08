open Kore;

module U = Util.ResultUtil;

let suite =
  "Grammar.Declaration | View"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.Declaration.parse_none([
          "gibberish",
          "view",
          "view foo",
          "view foo ()",
          "view foo () ->",
          "view foo () -> {",
          "view foo ->",
          "view foo -> {",
        ])
    ),
    "parse - inline with no arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            ([], [], U.nil_prim)
            |> Declaration.of_view
            |> U.as_view([], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo -> nil",
        )
    ),
    "parse - with body and no arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            (
              [],
              [],
              [U.nil_prim |> Statement.of_effect |> U.as_nil]
              |> Expression.of_closure
              |> U.as_nil,
            )
            |> Declaration.of_view
            |> U.as_view([], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo -> { nil }",
        )
    ),
    "parse - inline with empty arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            ([], [], U.nil_prim)
            |> Declaration.of_view
            |> U.as_view([], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo () -> nil",
        )
    ),
    "parse - with body and empty arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            (
              [],
              [],
              [U.nil_prim |> Statement.of_effect |> U.as_nil]
              |> Expression.of_closure
              |> U.as_nil,
            )
            |> Declaration.of_view
            |> U.as_view([], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo () -> { nil }",
        )
    ),
    "parse - with typed argument"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            (
              [
                (
                  U.as_untyped("fizz"),
                  Some(U.as_untyped(TypeExpression.Integer)),
                  None,
                )
                |> U.as_int,
              ],
              [],
              [] |> Expression.of_closure |> U.as_nil,
            )
            |> Declaration.of_view
            |> U.as_view([("fizz", (Valid(Integer), true))], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo (fizz: integer) -> {}",
        )
    ),
    "parse - with argument with default"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            (
              [
                (U.as_untyped("fizz"), None, Some(U.string_prim("bar")))
                |> U.as_string,
              ],
              [],
              [] |> Expression.of_closure |> U.as_nil,
            )
            |> Declaration.of_view
            |> U.as_view([("fizz", (Valid(String), false))], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo (fizz = \"bar\") -> {}",
        )
    ),
    "parse - with typed argument with default"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            (
              [
                (
                  U.as_untyped("fizz"),
                  Some(U.as_untyped(TypeExpression.Boolean)),
                  Some(U.bool_prim(true)),
                )
                |> U.as_bool,
              ],
              [],
              [] |> Expression.of_closure |> U.as_nil,
            )
            |> Declaration.of_view
            |> U.as_view([("fizz", (Valid(Boolean), false))], Valid(Nil)),
          )
          |> U.as_untyped,
          "view foo (fizz: boolean = true) -> {}",
        )
    ),
  ];
