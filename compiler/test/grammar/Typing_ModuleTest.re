open Kore;

module Typing = Grammar.Typing;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.module_t;

    let parser = _ =>
      Typing.module_parser |> Assert.parse_completely |> Parser.parse;

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
          U.as_raw_node(TD.Module(U.as_raw_node("Foo"), [])),
          "module Foo {}",
        )
    ),
    "parse module with declaration"
    >: (
      () =>
        Assert.parse(
          U.as_raw_node(
            TD.Module(
              U.as_raw_node("Foo"),
              [
                (U.as_raw_node("bar"), U.as_raw_node(TE.String))
                |> TD.of_declaration
                |> U.as_raw_node,
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
          U.as_raw_node(
            TD.Module(
              U.as_raw_node("Foo"),
              [
                (U.as_raw_node("bar"), U.as_raw_node(TE.Float))
                |> TD.of_type
                |> U.as_raw_node,
              ],
            ),
          ),
          "module Foo {
  type bar: float;
}",
        )
    ),
    "parse module with declarations and types"
    >: (
      () =>
        Assert.parse(
          U.as_raw_node(
            TD.Module(
              U.as_raw_node("Foo"),
              [
                (U.as_raw_node("foo"), U.as_raw_node(TE.Boolean))
                |> TD.of_type
                |> U.as_raw_node,
                (U.as_raw_node("bar"), U.as_raw_node(TE.Integer))
                |> TD.of_type
                |> U.as_raw_node,
                (U.as_raw_node("fizz"), U.as_raw_node(TE.Float))
                |> TD.of_declaration
                |> U.as_raw_node,
                (
                  U.as_raw_node("buzz"),
                  U.as_raw_node(
                    TE.Function(
                      [U.as_raw_node(TE.Element)],
                      U.as_raw_node(TE.String),
                    ),
                  ),
                )
                |> TD.of_declaration
                |> U.as_raw_node,
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
