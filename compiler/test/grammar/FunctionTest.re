open Kore;

module ExportKind = AST.ModuleStatement.ExportKind;

let suite =
  "Grammar.Function"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.Declaration.parse_none([
          "gibberish",
          "func",
          "func foo",
          "func foo ()",
          "func foo () ->",
          "func foo () -> {",
          "func foo ->",
          "func foo -> {",
        ])
    ),
    "parse - inline with no arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo -> nil",
        )
    ),
    "parse - with body and no arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            (
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo -> { nil }",
        )
    ),
    "parse - inline with empty arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo () -> nil",
        )
    ),
    "parse - with body and empty arguments"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            (
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo () -> { nil }",
        )
    ),
    "parse - with typed argument"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            (
              [
                (U.as_untyped("fizz"), Some(U.as_untyped(TE.Integer)), None)
                |> U.as_int,
              ],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([Valid(Integer)], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo (fizz: integer) -> {}",
        )
    ),
    "parse - with argument with default"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            (
              [
                (U.as_untyped("fizz"), None, Some(U.string_prim("bar")))
                |> U.as_string,
              ],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([Valid(String)], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo (fizz = \"bar\") -> {}",
        )
    ),
    "parse - with typed argument with default"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            (
              [
                (
                  U.as_untyped("fizz"),
                  Some(U.as_untyped(TE.Boolean)),
                  Some(U.bool_prim(true)),
                )
                |> U.as_bool,
              ],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([Valid(Boolean)], Valid(Nil)),
          )
          |> A.of_export
          |> U.as_untyped,
          "func foo (fizz: boolean = true) -> {}",
        )
    ),
  ];
