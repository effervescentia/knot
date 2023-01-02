open Kore;

module TE = AST.TypeExpression;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TE.t;

    let parser = _ =>
      KTypeExpression.Plugin.parse |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => KTypeExpression.Plugin.to_xml % Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Typing | Expression"
  >::: [
    "no parse" >: (() => Assert.no_parse("#!@e13")),
    "parse nil type" >: (() => Assert.parse(U.as_untyped(TE.Nil), "nil")),
    "parse boolean type"
    >: (() => Assert.parse(U.as_untyped(TE.Boolean), "boolean")),
    "parse integer type"
    >: (() => Assert.parse(U.as_untyped(TE.Integer), "integer")),
    "parse float type"
    >: (() => Assert.parse(U.as_untyped(TE.Float), "float")),
    "parse string type"
    >: (() => Assert.parse(U.as_untyped(TE.String), "string")),
    "parse element type"
    >: (() => Assert.parse(U.as_untyped(TE.Element), "element")),
    "parse style type"
    >: (() => Assert.parse(U.as_untyped(TE.Style), "style")),
    "parse type identifier"
    >: (
      () =>
        Assert.parse("foo" |> U.as_untyped |> TE.of_id |> U.as_untyped, "foo")
    ),
    "parse simple group type"
    >: (
      () =>
        Assert.parse_all(
          TE.Boolean |> U.as_untyped |> TE.of_group |> U.as_untyped,
          ["(boolean)", "( boolean )"],
        )
    ),
    "parse complex group type"
    >: (
      () =>
        Assert.parse(
          [
            (
              U.as_untyped("foo"),
              TE.Boolean
              |> U.as_untyped
              |> TE.of_list
              |> U.as_untyped
              |> TE.of_group
              |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
            (
              U.as_untyped("bar"),
              (
                [TE.Integer |> U.as_untyped |> TE.of_group |> U.as_untyped],
                TE.Nil |> U.as_untyped |> TE.of_group |> U.as_untyped,
              )
              |> TE.of_function
              |> U.as_untyped
              |> TE.of_group
              |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> U.as_untyped
          |> TE.of_group
          |> U.as_untyped,
          "({ foo: (boolean[]), bar: (((integer)) -> (nil)) })",
        )
    ),
    "parse empty struct type"
    >: (() => Assert.parse(U.as_untyped(TE.of_object([])), "{}")),
    "parse simple struct type"
    >: (
      () =>
        Assert.parse_all(
          [
            (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
            |> TE.of_required
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> U.as_untyped,
          ["{foo:boolean}", "{ foo : boolean }", "{ foo: boolean, }"],
        )
    ),
    "parse optional property struct type"
    >: (
      () =>
        Assert.parse(
          [
            (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TE.String))
            |> TE.of_optional
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> U.as_untyped,
          "{ foo: boolean, bar?: string }",
        )
    ),
    "parse complex struct type"
    >: (
      () =>
        Assert.parse(
          [
            (U.as_untyped("nil"), U.as_untyped(TE.Nil))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("boolean"), U.as_untyped(TE.Boolean))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("integer"), U.as_untyped(TE.Integer))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("float"), U.as_untyped(TE.Float))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("string"), U.as_untyped(TE.String))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("element"), U.as_untyped(TE.Element))
            |> TE.of_required
            |> U.as_untyped,
            (
              U.as_untyped("struct"),
              [
                (U.as_untyped("foo"), U.as_untyped(TE.Nil))
                |> TE.of_required
                |> U.as_untyped,
                (U.as_untyped("bar"), U.as_untyped(TE.Object([])))
                |> TE.of_required
                |> U.as_untyped,
              ]
              |> TE.of_object
              |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
            (
              U.as_untyped("function"),
              ([U.as_untyped(TE.Boolean)], U.as_untyped(TE.Integer))
              |> TE.of_function
              |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> U.as_untyped,
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
          ([], U.as_untyped(TE.Nil)) |> TE.of_function |> U.as_untyped,
          "() -> nil",
        )
    ),
    "parse simple function type"
    >: (
      () =>
        Assert.parse_all(
          (
            [U.as_untyped(TE.Boolean), U.as_untyped(TE.Float)],
            U.as_untyped(TE.Element),
          )
          |> TE.of_function
          |> U.as_untyped,
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
              [
                (U.as_untyped("foo"), U.as_untyped(TE.Nil))
                |> TE.of_required
                |> U.as_untyped,
              ]
              |> TE.of_object
              |> U.as_untyped,
              ([], U.as_untyped(TE.Nil)) |> TE.of_function |> U.as_untyped,
            ],
            ([U.as_untyped(TE.Element)], U.as_untyped(TE.Boolean))
            |> TE.of_function
            |> U.as_untyped,
          )
          |> TE.of_function
          |> U.as_untyped,
          "({ foo: nil }, () -> nil) -> (element) -> boolean",
        )
    ),
    "parse simple list type"
    >: (
      () =>
        Assert.parse_all(
          TE.Nil |> U.as_untyped |> TE.of_list |> U.as_untyped,
          ["nil[]", "nil [ ]"],
        )
    ),
    "parse multi-dimensional list type"
    >: (
      () =>
        Assert.parse(
          TE.Float
          |> U.as_untyped
          |> TE.of_list
          |> U.as_untyped
          |> TE.of_list
          |> U.as_untyped
          |> TE.of_list
          |> U.as_untyped,
          "float[][][]",
        )
    ),
    "parse complex list type"
    >: (
      () =>
        Assert.parse(
          [
            (
              U.as_untyped("foo"),
              TE.Nil |> U.as_untyped |> TE.of_list |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
            (
              U.as_untyped("bar"),
              (
                [TE.Float |> U.as_untyped |> TE.of_list |> U.as_untyped],
                TE.Integer |> U.as_untyped |> TE.of_list |> U.as_untyped,
              )
              |> TE.of_function
              |> U.as_untyped
              |> TE.of_group
              |> U.as_untyped
              |> TE.of_list
              |> U.as_untyped,
            )
            |> TE.of_required
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> U.as_untyped
          |> TE.of_list
          |> U.as_untyped,
          "{ foo: nil[], bar: ((float[]) -> integer[])[] }[]",
        )
    ),
    "parse nil view type"
    >: (
      () =>
        Assert.parse(
          ([] |> TE.of_object |> U.as_untyped, U.as_untyped(TE.Nil))
          |> TE.of_view
          |> U.as_untyped,
          "view({}, nil)",
        )
    ),
    "parse complex view type"
    >: (
      () =>
        Assert.parse(
          (
            [
              (U.as_untyped("foo"), U.as_untyped(TE.Nil))
              |> TE.of_required
              |> U.as_untyped,
            ]
            |> TE.of_object
            |> U.as_untyped,
            U.as_untyped(TE.Boolean),
          )
          |> TE.of_view
          |> U.as_untyped,
          "view({ foo: nil }, boolean)",
        )
    ),
  ];
