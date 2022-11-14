open Kore;

module A = AST.Result;
module T = AST.Type;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.ResultUtil;

module Assert =
  Assert.Make({
    type t = TD.module_t;

    let parser = _ =>
      Reference.Namespace.of_string("test_namespace")
      |> AST.ParseContext.create(
           ~symbols=
             AST.SymbolTable.of_export_list([
               (
                 Reference.Export.Named("fizz"),
                 T.Valid(`Decorator(([], Module))),
               ),
               (
                 Reference.Export.Named("buzz"),
                 T.Valid(
                   `Decorator((
                     [Valid(`Integer), Valid(`Boolean)],
                     Module,
                   )),
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
            ppf => KTypeDefinition.Plugin.module_to_xml % Fmt.xml_string(ppf),
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
          U.as_untyped(TD.Module(U.as_untyped("Foo"), [], [])),
          "module Foo {}",
        )
    ),
    "parse module with decorator"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            TD.Module(
              U.as_untyped("Foo"),
              [],
              [("fizz" |> U.as_decorator([], Module), []) |> U.as_untyped],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [],
              [
                (
                  "buzz"
                  |> U.as_decorator(
                       [Valid(`Integer), Valid(`Boolean)],
                       Module,
                     ),
                  [
                    123L |> A.of_int |> A.of_num |> U.as_int,
                    false |> A.of_bool |> U.as_bool,
                  ],
                )
                |> U.as_untyped,
              ],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [],
              [
                ("fizz" |> U.as_decorator([], Module), []) |> U.as_untyped,
                (
                  "buzz"
                  |> U.as_decorator(
                       [Valid(`Integer), Valid(`Boolean)],
                       Module,
                     ),
                  [
                    123L |> A.of_int |> A.of_num |> U.as_int,
                    false |> A.of_bool |> U.as_bool,
                  ],
                )
                |> U.as_untyped,
              ],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TE.String))
                |> TD.of_declaration
                |> U.as_untyped,
              ],
              [],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TE.Float))
                |> TD.of_type
                |> U.as_untyped,
              ],
              [],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (
                  U.as_untyped("bar"),
                  (
                    [
                      (U.as_untyped("foo"), U.as_untyped(TE.Integer))
                      |> TE.of_required
                      |> U.as_untyped,
                    ]
                    |> TE.of_struct
                    |> U.as_untyped,
                    U.as_untyped(TE.Element),
                  )
                  |> TE.of_view
                  |> U.as_untyped,
                )
                |> TD.of_type
                |> U.as_untyped,
              ],
              [],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (U.as_untyped("bar"), U.as_untyped(TE.Float))
                |> TD.of_type
                |> U.as_untyped,
                (
                  U.as_untyped("foo"),
                  "bar"
                  |> U.as_untyped
                  |> TE.of_id
                  |> U.as_untyped
                  |> TE.of_list
                  |> U.as_untyped,
                )
                |> TD.of_type
                |> U.as_untyped,
              ],
              [],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (
                  U.as_untyped("bar"),
                  [
                    (
                      U.as_untyped("Verified"),
                      [U.as_untyped(TE.Integer), U.as_untyped(TE.String)],
                    ),
                    (
                      U.as_untyped("Unverified"),
                      [U.as_untyped(TE.String)],
                    ),
                  ],
                )
                |> TD.of_enum
                |> U.as_untyped,
              ],
              [],
            ),
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
            TD.Module(
              U.as_untyped("Foo"),
              [
                (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
                |> TD.of_type
                |> U.as_untyped,
                (U.as_untyped("bar"), U.as_untyped(TE.Integer))
                |> TD.of_type
                |> U.as_untyped,
                (U.as_untyped("fizz"), U.as_untyped(TE.Float))
                |> TD.of_declaration
                |> U.as_untyped,
                (
                  U.as_untyped("buzz"),
                  U.as_untyped(
                    TE.Function(
                      [U.as_untyped(TE.Element)],
                      U.as_untyped(TE.String),
                    ),
                  ),
                )
                |> TD.of_declaration
                |> U.as_untyped,
              ],
              [],
            ),
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
