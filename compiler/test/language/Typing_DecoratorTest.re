open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TypeDefinition.node_t;

    let parser = _ =>
      Reference.Namespace.of_string("test_namespace")
      |> AST.ParseContext.create
      |> TypeDefinition.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => TypeDefinition.to_xml % Fmt.xml_string(ppf), (==)),
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
          (U.as_untyped("foo"), [], Module)
          |> TypeDefinition.of_decorator
          |> U.as_untyped,
          "decorator foo: () on module;",
        )
    ),
    "parse decorator with arguments"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("foo"),
            [
              U.as_untyped(TypeExpression.String),
              U.as_untyped(TypeExpression.Boolean),
              U.as_untyped(TypeExpression.Integer),
            ],
            Module,
          )
          |> TypeDefinition.of_decorator
          |> U.as_untyped,
          "decorator foo: (string, boolean, integer) on module;",
        )
    ),
  ];
