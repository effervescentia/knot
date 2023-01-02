open Kore;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((A.identifier_t, AM.declaration_t), unit);

    let parser = ctx =>
      (ctx, AST.Module.Named)
      |> KView.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, ((name, declaration), _)) =>
              KDeclaration.Plugin.to_xml(
                ~@T.pp,
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
  "Grammar.Declaration | View"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.parse_none([
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
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            ([], [], U.nil_prim) |> A.of_view |> U.as_view([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "view foo -> nil",
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
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "view foo -> { nil }",
        )
    ),
    "parse - inline with empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped,
            ([], [], U.nil_prim) |> A.of_view |> U.as_view([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "view foo () -> nil",
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
              [],
              [U.nil_prim |> A.of_effect |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view([], Valid(`Nil)),
          )
          |> U.as_untyped,
          "view foo () -> { nil }",
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
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view(
                 [("fizz", (Valid(`Integer), true))],
                 Valid(`Nil),
               ),
          )
          |> U.as_untyped,
          "view foo (fizz: integer) -> {}",
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
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view(
                 [("fizz", (Valid(`String), false))],
                 Valid(`Nil),
               ),
          )
          |> U.as_untyped,
          "view foo (fizz = \"bar\") -> {}",
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
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view(
                 [("fizz", (Valid(`Boolean), false))],
                 Valid(`Nil),
               ),
          )
          |> U.as_untyped,
          "view foo (fizz: boolean = true) -> {}",
        )
    ),
  ];
