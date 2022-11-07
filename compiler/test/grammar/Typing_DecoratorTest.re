open Kore;

module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.module_t;

    let parser = _ =>
      Reference.Namespace.of_string("test_namespace")
      |> AST.ParseContext.create
      |> KTypeDefinition.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              KTypeDefinition.Plugin.module_to_xml % Fmt.xml(Fmt.string, ppf),
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
