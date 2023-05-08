open Kore;

module U = Util.RawUtil;
module ObjectEntry = TypeExpression.ObjectEntry;

module Assert =
  Assert.Make({
    type t = TypeExpression.node_t;

    let parser = _ =>
      TypeExpression.parse |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => TypeExpression.to_xml % Fmt.xml_string(ppf), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Typing | Expression"
  >::: [
    "no parse" >: (() => Assert.no_parse("#!@e13")),
    "parse nil type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Nil), "nil")),
    "parse boolean type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Boolean), "boolean")),
    "parse integer type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Integer), "integer")),
    "parse float type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Float), "float")),
    "parse string type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.String), "string")),
    "parse element type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Element), "element")),
    "parse style type"
    >: (() => Assert.parse(U.as_untyped(TypeExpression.Style), "style")),
    "parse type identifier"
    >: (
      () =>
        Assert.parse(
          "foo" |> U.as_untyped |> TypeExpression.of_identifier |> U.as_untyped,
          "foo",
        )
    ),
    "parse simple group type"
    >: (
      () =>
        Assert.parse_all(
          TypeExpression.Boolean
          |> U.as_untyped
          |> TypeExpression.of_group
          |> U.as_untyped,
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
              TypeExpression.Boolean
              |> U.as_untyped
              |> TypeExpression.of_list
              |> U.as_untyped
              |> TypeExpression.of_group
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (
              U.as_untyped("bar"),
              (
                [
                  TypeExpression.Integer
                  |> U.as_untyped
                  |> TypeExpression.of_group
                  |> U.as_untyped,
                ],
                TypeExpression.Nil
                |> U.as_untyped
                |> TypeExpression.of_group
                |> U.as_untyped,
              )
              |> TypeExpression.of_function
              |> U.as_untyped
              |> TypeExpression.of_group
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
          |> U.as_untyped
          |> TypeExpression.of_group
          |> U.as_untyped,
          "({ foo: (boolean[]), bar: (((integer)) -> (nil)) })",
        )
    ),
    "parse empty struct type"
    >: (
      () => Assert.parse(U.as_untyped(TypeExpression.of_object([])), "{}")
    ),
    "parse simple struct type"
    >: (
      () =>
        Assert.parse_all(
          [
            (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
            |> ObjectEntry.of_required
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
          |> U.as_untyped,
          ["{foo:boolean}", "{ foo : boolean }", "{ foo: boolean, }"],
        )
    ),
    "parse optional property struct type"
    >: (
      () =>
        Assert.parse(
          [
            (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
            |> ObjectEntry.of_optional
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
          |> U.as_untyped,
          "{ foo: boolean, bar?: string }",
        )
    ),
    "parse complex struct type"
    >: (
      () =>
        Assert.parse(
          [
            (U.as_untyped("nil"), U.as_untyped(TypeExpression.Nil))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("boolean"), U.as_untyped(TypeExpression.Boolean))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("integer"), U.as_untyped(TypeExpression.Integer))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("float"), U.as_untyped(TypeExpression.Float))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("string"), U.as_untyped(TypeExpression.String))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("element"), U.as_untyped(TypeExpression.Element))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (
              U.as_untyped("struct"),
              [
                (U.as_untyped("foo"), U.as_untyped(TypeExpression.Nil))
                |> ObjectEntry.of_required
                |> U.as_untyped,
                (
                  U.as_untyped("bar"),
                  U.as_untyped(TypeExpression.Object([])),
                )
                |> ObjectEntry.of_required
                |> U.as_untyped,
              ]
              |> TypeExpression.of_object
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (
              U.as_untyped("function"),
              (
                [U.as_untyped(TypeExpression.Boolean)],
                U.as_untyped(TypeExpression.Integer),
              )
              |> TypeExpression.of_function
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
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
          ([], U.as_untyped(TypeExpression.Nil))
          |> TypeExpression.of_function
          |> U.as_untyped,
          "() -> nil",
        )
    ),
    "parse simple function type"
    >: (
      () =>
        Assert.parse_all(
          (
            [
              U.as_untyped(TypeExpression.Boolean),
              U.as_untyped(TypeExpression.Float),
            ],
            U.as_untyped(TypeExpression.Element),
          )
          |> TypeExpression.of_function
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
                (U.as_untyped("foo"), U.as_untyped(TypeExpression.Nil))
                |> ObjectEntry.of_required
                |> U.as_untyped,
              ]
              |> TypeExpression.of_object
              |> U.as_untyped,
              ([], U.as_untyped(TypeExpression.Nil))
              |> TypeExpression.of_function
              |> U.as_untyped,
            ],
            (
              [U.as_untyped(TypeExpression.Element)],
              U.as_untyped(TypeExpression.Boolean),
            )
            |> TypeExpression.of_function
            |> U.as_untyped,
          )
          |> TypeExpression.of_function
          |> U.as_untyped,
          "({ foo: nil }, () -> nil) -> (element) -> boolean",
        )
    ),
    "parse simple list type"
    >: (
      () =>
        Assert.parse_all(
          TypeExpression.Nil
          |> U.as_untyped
          |> TypeExpression.of_list
          |> U.as_untyped,
          ["nil[]", "nil [ ]"],
        )
    ),
    "parse multi-dimensional list type"
    >: (
      () =>
        Assert.parse(
          TypeExpression.Float
          |> U.as_untyped
          |> TypeExpression.of_list
          |> U.as_untyped
          |> TypeExpression.of_list
          |> U.as_untyped
          |> TypeExpression.of_list
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
              TypeExpression.Nil
              |> U.as_untyped
              |> TypeExpression.of_list
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (
              U.as_untyped("bar"),
              (
                [
                  TypeExpression.Float
                  |> U.as_untyped
                  |> TypeExpression.of_list
                  |> U.as_untyped,
                ],
                TypeExpression.Integer
                |> U.as_untyped
                |> TypeExpression.of_list
                |> U.as_untyped,
              )
              |> TypeExpression.of_function
              |> U.as_untyped
              |> TypeExpression.of_group
              |> U.as_untyped
              |> TypeExpression.of_list
              |> U.as_untyped,
            )
            |> ObjectEntry.of_required
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
          |> U.as_untyped
          |> TypeExpression.of_list
          |> U.as_untyped,
          "{ foo: nil[], bar: ((float[]) -> integer[])[] }[]",
        )
    ),
    "parse nil view type"
    >: (
      () =>
        Assert.parse(
          (
            [] |> TypeExpression.of_object |> U.as_untyped,
            U.as_untyped(TypeExpression.Nil),
          )
          |> TypeExpression.of_view
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
              (U.as_untyped("foo"), U.as_untyped(TypeExpression.Nil))
              |> ObjectEntry.of_required
              |> U.as_untyped,
            ]
            |> TypeExpression.of_object
            |> U.as_untyped,
            U.as_untyped(TypeExpression.Boolean),
          )
          |> TypeExpression.of_view
          |> U.as_untyped,
          "view({ foo: nil }, boolean)",
        )
    ),
  ];
