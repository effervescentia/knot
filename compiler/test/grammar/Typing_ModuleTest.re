open Kore;

module Typing = Grammar.Typing;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.module_t;

    let parser = _ =>
      Typing.module_parser(
        NamespaceContext2.create(Namespace.of_string("test_namespace")),
      )
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, type_module) =>
              A.Dump.(
                type_module
                |> untyped_node_to_entity(
                     ~children=[TD.Dump.to_entity(type_module)],
                     "TypeModule",
                   )
                |> Entity.pp(ppf)
              ),
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
          U.as_untyped(TD.Module(U.as_untyped("Foo"), [])),
          "module Foo {}",
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
            ),
          ),
          "module Foo {
  decl bar: string;
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
            ),
          ),
          "module Foo {
  type bar: float;
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
            ),
          ),
          "module Foo {
  type foo: boolean;
  type bar: integer;

  decl fizz: float;
  decl buzz: (element) -> string;
}",
        )
    ),
  ];
