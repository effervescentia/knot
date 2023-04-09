open Kore;

module U = Util.ResultUtil;

module Assert =
  Assert.Make({
    type t = TypeDefinition.node_t;

    let parser = _ =>
      Reference.Namespace.of_string("test_namespace")
      |> AST.ParseContext.create(
           ~symbols=
             AST.SymbolTable.of_export_list([
               (Export.Named("fizz"), Type.Valid(Decorator([], Module))),
               (
                 Export.Named("buzz"),
                 Type.Valid(
                   Decorator([Valid(Integer), Valid(Boolean)], Module),
                 ),
               ),
             ]),
         )
      |> KTypeDefinition.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => KTypeDefinition.Debug.to_xml % Fmt.xml_string(ppf),
            (==),
          ),
          "type definition matches",
        )
      );
  });

let suite =
  "Grammar.Typing | Module"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse empty module"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (U.as_untyped("Foo"), [], []) |> TypeDefinition.of_module,
          ),
          "module Foo {}",
        )
    ),
    "parse module with decorator"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [],
              [("fizz" |> U.as_decorator([], Module), []) |> U.as_untyped],
            )
            |> TypeDefinition.of_module,
          ),
          "@fizz
module Foo {}",
        )
    ),
    "parse module with decorator and arguments"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [],
              [
                (
                  "buzz"
                  |> U.as_decorator(
                       [Valid(Integer), Valid(Boolean)],
                       Module,
                     ),
                  [
                    123L |> Primitive.of_integer |> U.as_int,
                    false |> Primitive.of_boolean |> U.as_bool,
                  ],
                )
                |> U.as_untyped,
              ],
            )
            |> TypeDefinition.of_module,
          ),
          "@buzz(123, false)
module Foo {}",
        )
    ),
    "parse module with multiple decorators"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [],
              [
                ("fizz" |> U.as_decorator([], Module), []) |> U.as_untyped,
                (
                  "buzz"
                  |> U.as_decorator(
                       [Valid(Integer), Valid(Boolean)],
                       Module,
                     ),
                  [
                    123L |> Primitive.of_integer |> U.as_int,
                    false |> Primitive.of_boolean |> U.as_bool,
                  ],
                )
                |> U.as_untyped,
              ],
            )
            |> TypeDefinition.of_module,
          ),
          "@fizz
@buzz(123, false)
module Foo {}",
        )
    ),
    "parse module with declaration"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
                |> TypeStatement.of_declaration
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  declare bar: string;
}",
        )
    ),
    "parse module with type"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TypeExpression.Float))
                |> TypeStatement.of_type
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  type bar: float;
}",
        )
    ),
    "parse module with view type"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (
                  U.as_untyped("bar"),
                  (
                    [
                      (
                        U.as_untyped("foo"),
                        U.as_untyped(TypeExpression.Integer),
                      )
                      |> TypeExpression.ObjectEntry.of_required
                      |> U.as_untyped,
                    ]
                    |> TypeExpression.of_object
                    |> U.as_untyped,
                    U.as_untyped(TypeExpression.Element),
                  )
                  |> TypeExpression.of_view
                  |> U.as_untyped,
                )
                |> TypeStatement.of_type
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  type bar: view({ foo: integer }, element);
}",
        )
    ),
    "parse module with dependent types"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TypeExpression.Float))
                |> TypeStatement.of_type
                |> U.as_untyped,
                (
                  U.as_untyped("foo"),
                  "bar"
                  |> U.as_untyped
                  |> TypeExpression.of_identifier
                  |> U.as_untyped
                  |> TypeExpression.of_list
                  |> U.as_untyped,
                )
                |> TypeStatement.of_type
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  type bar: float;
  type foo: bar[];
}",
        )
    ),
    "parse module with enum"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (
                  U.as_untyped("bar"),
                  [
                    (
                      U.as_untyped("Verified"),
                      [
                        U.as_untyped(TypeExpression.Integer),
                        U.as_untyped(TypeExpression.String),
                      ],
                    ),
                    (
                      U.as_untyped("Unverified"),
                      [U.as_untyped(TypeExpression.String)],
                    ),
                  ],
                )
                |> TypeStatement.of_enumerated
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  enum bar:
    | Verified(integer, string)
    | Unverified(string);
}",
        )
    ),
    "parse module with declarations and types"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            (
              U.as_untyped("Foo"),
              [
                (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
                |> TypeStatement.of_type
                |> U.as_untyped,
                (U.as_untyped("bar"), U.as_untyped(TypeExpression.Integer))
                |> TypeStatement.of_type
                |> U.as_untyped,
                (U.as_untyped("fizz"), U.as_untyped(TypeExpression.Float))
                |> TypeStatement.of_declaration
                |> U.as_untyped,
                (
                  U.as_untyped("buzz"),
                  U.as_untyped(
                    (
                      [U.as_untyped(TypeExpression.Element)],
                      U.as_untyped(TypeExpression.String),
                    )
                    |> TypeExpression.of_function,
                  ),
                )
                |> TypeStatement.of_declaration
                |> U.as_untyped,
              ],
              [],
            )
            |> TypeDefinition.of_module,
          ),
          "module Foo {
  type foo: boolean;
  type bar: integer;

  declare fizz: float;
  declare buzz: (element) -> string;
}",
        )
    ),
  ];
