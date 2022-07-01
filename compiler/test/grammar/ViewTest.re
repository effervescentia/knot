open Kore;
open Reference;

module View = Grammar.View;
module U = Util.ResultUtil;
module TE = A.TypeExpression;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((A.export_t, A.declaration_t), unit);

    let parser = ctx =>
      View.parser(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = fst(stmt);

              A.Dump.(
                untyped_node_to_entity(
                  "Declaration",
                  ~children=[
                    export |> export_to_entity,
                    decl |> decl_to_entity,
                  ],
                  stmt,
                )
                |> Entity.pp(ppf)
              );
            },
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
            "foo" |> U.as_untyped |> A.of_named_export,
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
            "foo" |> U.as_untyped |> A.of_named_export,
            (
              [],
              [],
              [U.nil_prim |> A.of_expr |> U.as_nil]
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
            "foo" |> U.as_untyped |> A.of_named_export,
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
            "foo" |> U.as_untyped |> A.of_named_export,
            (
              [],
              [],
              [U.nil_prim |> A.of_expr |> U.as_nil]
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
            "foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: Some(U.as_untyped(TE.Integer)),
                  default: None,
                }
                |> U.as_int,
              ],
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view([("fizz", Valid(`Integer))], Valid(`Nil)),
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
            "foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: None,
                  default: Some(U.string_prim("bar")),
                }
                |> U.as_string,
              ],
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view([("fizz", Valid(`String))], Valid(`Nil)),
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
            "foo" |> U.as_untyped |> A.of_named_export,
            (
              [
                A.{
                  name: U.as_untyped("fizz"),
                  type_: Some(U.as_untyped(TE.Boolean)),
                  default: Some(U.bool_prim(true)),
                }
                |> U.as_bool,
              ],
              [],
              [] |> A.of_closure |> U.as_nil,
            )
            |> A.of_view
            |> U.as_view([("fizz", Valid(`Boolean))], Valid(`Nil)),
          )
          |> U.as_untyped,
          "view foo (fizz: boolean = true) -> {}",
        )
    ),
  ];
