open Kore;

module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = TD.decorator_t(TR.t);

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
                decorator |> decorator_to_entity(TR.pp) |> Entity.pp(ppf)
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
    >: (
      () => Assert.parse((U.as_unknown("foo"), []) |> U.as_untyped, "@foo")
    ),
    "parse with empty arguments"
    >: (
      () =>
        Assert.parse((U.as_unknown("foo"), []) |> U.as_untyped, "@foo()")
    ),
    "parse with arguments"
    >: (
      () =>
        Assert.parse(
          (
            U.as_unknown("foo"),
            [
              123L |> AR.of_int |> AR.of_num |> U.as_int,
              true |> AR.of_bool |> U.as_bool,
              "bar" |> AR.of_string |> U.as_string,
            ],
          )
          |> U.as_untyped,
          "@foo(123, true, \"bar\")",
        )
    ),
  ];
