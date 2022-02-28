open Kore;
open Reference;

module Declaration = Grammar.Declaration;
module U = Util.ResultUtilV2;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = Node.Raw.t((A.export_t, A.declaration_t));

    let parser = ((_, ctx)) =>
      Declaration.function_(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = Node.Raw.get_value(stmt);

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
  "Grammar.Declaration | Function"
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
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_raw_node,
          "func foo -> nil",
        )
    ),
    "parse - with body and no arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            (
              [],
              [U.nil_prim |> A.of_expr |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_raw_node,
          "func foo -> { nil }",
        )
    ),
    "parse - inline with empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            ([], U.nil_prim) |> A.of_func |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_raw_node,
          "func foo () -> nil",
        )
    ),
    "parse - with body and empty arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            (
              [],
              [U.nil_prim |> A.of_expr |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_raw_node,
          "func foo () -> { nil }",
        )
    ),
    "parse - with arguments"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            (
              [
                A.{
                  name: "fizz" |> A.of_public |> U.as_raw_node,
                  type_: None,
                  default: None,
                }
                |> U.as_int,
              ],
              [U.nil_prim |> A.of_expr |> U.as_nil]
              |> A.of_closure
              |> U.as_nil,
            )
            |> A.of_func
            |> U.as_function([], Valid(`Nil)),
          )
          |> U.as_raw_node,
          "func foo (fizz, buzz) -> {}",
        )
    ),
  ];
