open Kore;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((AST.Common.identifier_t, AM.declaration_t), unit);

    let parser = ctx =>
      (ctx, AST.Module.Named)
      |> KFunction.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, ((name, declaration), _)) =>
              KDeclaration.Plugin.to_xml(
                ~@AST.Type.pp,
                (AST.Module.Named, name, declaration),
              )
              |> Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Function"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.parse_none([
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
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo -> nil",
        )
    ),
    "parse - with body and no arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            (
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo -> { nil }",
        )
    ),
    "parse - inline with empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo () -> nil",
        )
    ),
    "parse - with body and empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            (
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo () -> { nil }",
        )
    ),
    "parse - with typed argument"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            (
              [
                (U.as_untyped("fizz"), Some(U.as_untyped(TE.Integer)), None)
                |> U.as_int,
              ],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([Valid(`Integer)], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo (fizz: integer) -> {}",
        )
    ),
    "parse - with argument with default"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            (
              [
                (U.as_untyped("fizz"), None, Some(U.string_prim("bar")))
                |> U.as_string,
              ],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([Valid(`String)], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo (fizz = \"bar\") -> {}",
        )
    ),
    "parse - with typed argument with default"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
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
            |> U.as_function([Valid(`Boolean)], Valid(`Nil)),
          )
          |> U.as_untyped,
          "func foo (fizz: boolean = true) -> {}",
        )
    ),
  ];
