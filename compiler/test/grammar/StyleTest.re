open Kore;
open Reference;

module Style = Grammar.Style;
module Expression = Grammar.Expression;
module U = Util.RawUtil;
module TE = A.TypeExpression;

let __context = ParseContext.create(Namespace.Internal("foo"));

module Assert =
  Assert.Make({
    type t = AR.expression_t;

    let parser = ctx =>
      Style.parser(ctx, Expression.parser)
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
    >: (() => Assert.parse([] |> AR.of_style |> U.as_style, "style { }")),
    "parse - with one rule"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [
            ("color" |> U.as_unknown, "$red" |> AR.of_id |> U.as_unknown)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_style,
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
            ("color" |> U.as_unknown, "$red" |> AR.of_id |> U.as_unknown)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_style,
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
              "height" |> U.as_unknown,
              (
                "$px" |> AR.of_id |> U.as_unknown,
                [(20.0, 2) |> U.float_prim],
              )
              |> AR.of_func_call
              |> U.as_unknown,
            )
            |> U.as_untyped,
            ("color" |> U.as_unknown, "$red" |> AR.of_id |> U.as_unknown)
            |> U.as_untyped,
          ]
          |> AR.of_style
          |> U.as_style,
          "style {
            height: $px(20.0),
            color: $red
          }",
        )
    ),
  ];
