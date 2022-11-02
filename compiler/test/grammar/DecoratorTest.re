open Kore;

module AR = AST.Raw;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.decorator_t(unit);

    let parser = _ =>
      KPrimitive.Plugin.parse
      |> KDecorator.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, decorator) =>
              TD.Dump.(
                decorator
                |> decorator_to_entity((_, ()) => ())
                |> Entity.pp(ppf)
              ),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Decorator"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse without arguments"
    >: (() => Assert.parse((U.as_node("foo"), []) |> U.as_untyped, "@foo")),
    "parse with empty arguments"
    >: (
      () => Assert.parse((U.as_node("foo"), []) |> U.as_untyped, "@foo()")
    ),
    "parse with arguments"
    >: (
      () =>
        Assert.parse(
          (
            U.as_node("foo"),
            [
              123L |> AR.of_int |> AR.of_num |> U.as_node,
              true |> AR.of_bool |> U.as_node,
              "bar" |> AR.of_string |> U.as_node,
            ],
          )
          |> U.as_untyped,
          "@foo(123, true, \"bar\")",
        )
    ),
  ];
