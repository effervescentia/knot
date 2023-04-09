open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AST.Common.raw_t(KDecorator.Interface.t(Primitive.t, unit));

    let parser = _ =>
      Primitive.Parser.parse_primitive
      |> KDecorator.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              KDecorator.Plugin.to_xml((
                Dump.node_to_xml(
                  ~unpack=Primitive.Debug.primitive_to_xml % List.single,
                  "Value",
                ),
                _ => "Unknown",
              ))
              % Fmt.xml_string(ppf),
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
              123L |> Primitive.of_integer |> U.as_node,
              true |> Primitive.of_boolean |> U.as_node,
              "bar" |> Primitive.of_string |> U.as_node,
            ],
          )
          |> U.as_untyped,
          "@foo(123, true, \"bar\")",
        )
    ),
  ];
