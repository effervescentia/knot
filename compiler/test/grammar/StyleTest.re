open Kore;
open Reference;

module AR = AST.Raw;
module U = Util.RawUtil;
module TE = AST.TypeExpression;

let __context = AST.ParseContext.create(Namespace.Internal("foo"));

module Assert =
  Assert.Make({
    type t = AR.expression_t;

    let parser = ctx =>
      KStyle.Plugin.parse(ctx, KExpression.Plugin.parse)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => AR.Dump.(expr_to_entity % Entity.pp(ppf)), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Style"
  >::: [
    "no parse" >: (() => Assert.parse_none(["gibberish", "style", "style {"])),
    "parse - with no arguments"
    >: (() => Assert.parse([] |> AR.of_style |> U.as_node, "style { }")),
    "parse - with one rule"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [
            ("color" |> U.as_node, "$red" |> AR.of_id |> U.as_node)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_node,
          "style {
            color: $red
          }",
        )
    ),
    "parse - with trailing comma"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [
            ("color" |> U.as_node, "$red" |> AR.of_id |> U.as_node)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_node,
          "style {
            color: $red,
          }",
        )
    ),
    "parse - with multiple rules"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [
            (
              "height" |> U.as_node,
              ("$px" |> AR.of_id |> U.as_node, [(20.0, 2) |> U.float_prim])
              |> AR.of_func_call
              |> U.as_node,
            )
            |> U.as_untyped,
            ("color" |> U.as_node, "$red" |> AR.of_id |> U.as_node)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_node,
          "style {
            height: $px(20.0),
            color: $red
          }",
        )
    ),
  ];
