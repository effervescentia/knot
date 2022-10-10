open Kore;
open Reference;

module Style = Grammar.Style;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;
module TE = A.TypeExpression;

let __style_rule_type =
  T.Valid(`Function(([Valid(`String)], Valid(`Nil))));
let __module_table =
  ModuleTable.create(
    ~plugins=[
      (
        Plugin.StyleRule,
        [
          ("height", Value(__style_rule_type)),
          ("color", Value(__style_rule_type)),
        ],
      ),
      (
        Plugin.StyleExpression,
        [
          (
            "px",
            Value(
              T.Valid(`Function(([Valid(`Integer)], Valid(`String)))),
            ),
          ),
          ("red", Value(T.Valid(`String))),
        ],
      ),
    ],
    0,
  );
let __context =
  ParseContext.create(~modules=__module_table, Namespace.Internal("foo"));

module Assert =
  Assert.Make({
    type t = AR.expression_t;

    let parser = Style.parser % Assert.parse_completely % Parser.parse;

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
    >: (() => Assert.parse([] |> AR.of_style |> URaw.as_style, "style { }")),
    "parse - with one rule"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [
            (
              "color" |> URes.as_typed(__style_rule_type),
              "$red" |> AR.of_id |> URaw.as_unknown,
            )
            |> URaw.as_untyped,
          ]
          |> AR.of_style
          |> URaw.as_style,
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
            (
              "color" |> URes.as_typed(__style_rule_type),
              "$red" |> AR.of_id |> URaw.as_unknown,
            )
            |> URaw.as_untyped,
          ]
          |> AR.of_style
          |> URaw.as_style,
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
              "height" |> URes.as_typed(__style_rule_type),
              (
                "$px" |> AR.of_id |> URaw.as_unknown,
                [(20.0, 2) |> URaw.float_prim],
              )
              |> AR.of_func_call
              |> URaw.as_unknown,
            )
            |> URaw.as_untyped,
            (
              "color" |> URes.as_typed(__style_rule_type),
              "$red" |> AR.of_id |> URaw.as_unknown,
            )
            |> URaw.as_untyped,
          ]
          |> AR.of_style
          |> URaw.as_style,
          "style {
            height: $px(20.0),
            color: $red
          }",
        )
    ),
  ];
