open Kore;

module Typing = Grammar.Typing;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.module_t;

    let parser = _ =>
      Namespace.of_string("test_namespace")
      |> ParseContext.create
      |> Typing.root_parser
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
                     "Decorator",
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
  "Grammar.Typing | Decorator"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse decorator with no arguments"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(TD.Decorator(U.as_untyped("foo"), [], Module)),
          "decorator foo: () on module;",
        )
    ),
    "parse decorator with arguments"
    >: (
      () =>
        Assert.parse(
          U.as_untyped(
            TD.Decorator(
              U.as_untyped("foo"),
              [
                U.as_untyped(TE.String),
                U.as_untyped(TE.Boolean),
                U.as_untyped(TE.Integer),
              ],
              Module,
            ),
          ),
          "decorator foo: (string, boolean, integer) on module;",
        )
    ),
  ];