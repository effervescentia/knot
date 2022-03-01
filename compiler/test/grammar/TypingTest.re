open Kore;

module Typing = Grammar.Typing;
module TE = AST.TypeExpression;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TE.t;

    let parser = _ =>
      Typing.expression_parser |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, type_expr) =>
              A.Dump.(
                type_expr
                |> untyped_node_to_entity(
                     ~children=[TE.Dump.to_entity(type_expr)],
                     "TypeExpression",
                   )
                |> Entity.pp(ppf)
              ),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Typing"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse nil type" >: (() => Assert.parse(U.as_raw_node(TE.Nil), "nil")),
    "parse boolean type"
    >: (() => Assert.parse(U.as_raw_node(TE.Boolean), "boolean")),
    "parse integer type"
    >: (() => Assert.parse(U.as_raw_node(TE.Integer), "integer")),
    "parse float type"
    >: (() => Assert.parse(U.as_raw_node(TE.Float), "float")),
    "parse string type"
    >: (() => Assert.parse(U.as_raw_node(TE.String), "string")),
    "parse element type"
    >: (() => Assert.parse(U.as_raw_node(TE.Element), "element")),
    "parse simple group type"
    >: (
      () =>
        Assert.parse_all(
          TE.Boolean |> U.as_raw_node |> TE.of_group |> U.as_raw_node,
          ["(boolean)", "( boolean )"],
        )
    ),
    "parse complex group type"
    >: (
      () =>
        Assert.parse(
          [
            (
              U.as_raw_node("foo"),
              TE.Boolean
              |> U.as_raw_node
              |> TE.of_list
              |> U.as_raw_node
              |> TE.of_group
              |> U.as_raw_node,
            ),
            (
              U.as_raw_node("bar"),
              (
                [TE.Integer |> U.as_raw_node |> TE.of_group |> U.as_raw_node],
                TE.Nil |> U.as_raw_node |> TE.of_group |> U.as_raw_node,
              )
              |> TE.of_function
              |> U.as_raw_node
              |> TE.of_group
              |> U.as_raw_node,
            ),
          ]
          |> TE.of_struct
          |> U.as_raw_node
          |> TE.of_group
          |> U.as_raw_node,
          "({ foo: (boolean[]), bar: (((integer)) -> (nil)) })",
        )
    ),
    "parse empty struct type"
    >: (() => Assert.parse(U.as_raw_node(TE.of_struct([])), "{}")),
    "parse simple struct type"
    >: (
      () =>
        Assert.parse_all(
          [(U.as_raw_node("foo"), U.as_raw_node(TE.Boolean))]
          |> TE.of_struct
          |> U.as_raw_node,
          ["{foo:boolean}", "{ foo : boolean }", "{ foo: boolean, }"],
        )
    ),
    "parse complex struct type"
    >: (
      () =>
        Assert.parse(
          [
            (U.as_raw_node("nil"), U.as_raw_node(TE.Nil)),
            (U.as_raw_node("boolean"), U.as_raw_node(TE.Boolean)),
            (U.as_raw_node("integer"), U.as_raw_node(TE.Integer)),
            (U.as_raw_node("float"), U.as_raw_node(TE.Float)),
            (U.as_raw_node("string"), U.as_raw_node(TE.String)),
            (U.as_raw_node("element"), U.as_raw_node(TE.Element)),
            (
              U.as_raw_node("struct"),
              [
                (U.as_raw_node("foo"), U.as_raw_node(TE.Nil)),
                (U.as_raw_node("bar"), U.as_raw_node(TE.Struct([]))),
              ]
              |> TE.of_struct
              |> U.as_raw_node,
            ),
            (
              U.as_raw_node("function"),
              ([U.as_raw_node(TE.Boolean)], U.as_raw_node(TE.Integer))
              |> TE.of_function
              |> U.as_raw_node,
            ),
          ]
          |> TE.of_struct
          |> U.as_raw_node,
          "{
            nil: nil,
            boolean: boolean,
            integer: integer,
            float: float,
            string: string,
            element: element,
            struct: {
              foo: nil,
              bar: {}
            },
            function: (boolean) -> integer,
          }",
        )
    ),
    "parse nil function type"
    >: (
      () =>
        Assert.parse(
          ([], U.as_raw_node(TE.Nil)) |> TE.of_function |> U.as_raw_node,
          "() -> nil",
        )
    ),
    "parse simple function type"
    >: (
      () =>
        Assert.parse_all(
          (
            [U.as_raw_node(TE.Boolean), U.as_raw_node(TE.Float)],
            U.as_raw_node(TE.Element),
          )
          |> TE.of_function
          |> U.as_raw_node,
          [
            "(boolean,float)->element",
            "( boolean , float ) -> element",
            "(boolean, float,) -> element",
          ],
        )
    ),
    "parse complex function type"
    >: (
      () =>
        Assert.parse(
          (
            [
              [(U.as_raw_node("foo"), U.as_raw_node(TE.Nil))]
              |> TE.of_struct
              |> U.as_raw_node,
              ([], U.as_raw_node(TE.Nil)) |> TE.of_function |> U.as_raw_node,
            ],
            ([U.as_raw_node(TE.Element)], U.as_raw_node(TE.Boolean))
            |> TE.of_function
            |> U.as_raw_node,
          )
          |> TE.of_function
          |> U.as_raw_node,
          "({ foo: nil }, () -> nil) -> (element) -> boolean",
        )
    ),
    "parse simple list type"
    >: (
      () =>
        Assert.parse_all(
          TE.Nil |> U.as_raw_node |> TE.of_list |> U.as_raw_node,
          ["nil[]", "nil [ ]"],
        )
    ),
    "parse multi-dimensional list type"
    >: (
      () =>
        Assert.parse(
          TE.Float
          |> U.as_raw_node
          |> TE.of_list
          |> U.as_raw_node
          |> TE.of_list
          |> U.as_raw_node
          |> TE.of_list
          |> U.as_raw_node,
          "float[][][]",
        )
    ),
    "parse complex list type"
    >: (
      () =>
        Assert.parse(
          [
            (
              U.as_raw_node("foo"),
              TE.Nil |> U.as_raw_node |> TE.of_list |> U.as_raw_node,
            ),
            (
              U.as_raw_node("bar"),
              (
                [TE.Float |> U.as_raw_node |> TE.of_list |> U.as_raw_node],
                TE.Integer |> U.as_raw_node |> TE.of_list |> U.as_raw_node,
              )
              |> TE.of_function
              |> U.as_raw_node
              |> TE.of_group
              |> U.as_raw_node
              |> TE.of_list
              |> U.as_raw_node,
            ),
          ]
          |> TE.of_struct
          |> U.as_raw_node
          |> TE.of_list
          |> U.as_raw_node,
          "{ foo: nil[], bar: ((float[]) -> integer[])[] }[]",
        )
    ),
  ];
