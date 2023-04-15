open Kore;
open Reference;

module U = Util.RawUtil;

let __context = ParseContext.create(Namespace.Internal("foo"));

let suite =
  "Grammar.Style"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.Expression.parse_throws_compiler_errs(
          [
            ParseError(
              ReservedKeyword("style"),
              Internal("mock"),
              Range.zero,
            ),
          ],
          "style {",
        )
    ),
    "parse - with no arguments"
    >: (
      () =>
        Assert.Expression.parse(
          [] |> Expression.of_style |> U.as_node,
          "style { }",
        )
    ),
    "parse - with one rule"
    >: (
      () =>
        Assert.Expression.parse(
          ~context=__context,
          [
            (
              "color" |> U.as_node,
              "$red" |> Expression.of_identifier |> U.as_node,
            )
            |> U.as_untyped,
          ]
          |> Expression.of_style
          |> U.as_node,
          "style {
            color: $red
          }",
        )
    ),
    "parse - with trailing comma"
    >: (
      () =>
        Assert.Expression.parse(
          ~context=__context,
          [
            (
              "color" |> U.as_node,
              "$red" |> Expression.of_identifier |> U.as_node,
            )
            |> U.as_untyped,
          ]
          |> Expression.of_style
          |> U.as_node,
          "style {
            color: $red,
          }",
        )
    ),
    "parse - with multiple rules"
    >: (
      () =>
        Assert.Expression.parse(
          ~context=__context,
          [
            (
              "height" |> U.as_node,
              (
                "$px" |> Expression.of_identifier |> U.as_node,
                [(20.0, 2) |> U.float_prim],
              )
              |> Expression.of_function_call
              |> U.as_node,
            )
            |> U.as_untyped,
            (
              "color" |> U.as_node,
              "$red" |> Expression.of_identifier |> U.as_node,
            )
            |> U.as_untyped,
          ]
          |> Expression.of_style
          |> U.as_node,
          "style {
            height: $px(20.0),
            color: $red
          }",
        )
    ),
  ];
