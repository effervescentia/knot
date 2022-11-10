open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.identifier_t;

    let parser =
      KIdentifier.Plugin.parse_id % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Dump.node_to_xml(~dump_value=Fun.id, "Identifier")
              % Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Identifier"
  >::: [
    "no parse" >: (() => Assert.no_parse("~gibberish")),
    "parse public identifier"
    >: (() => Assert.parse(U.as_untyped("foo"), "foo")),
    "throw error on reserved keywords"
    >: (
      () =>
        Constants.Keyword.reserved
        |> List.iter(keyword =>
             Assert.parse_throws(
               AST.Error.CompileError([
                 ParseError(
                   ReservedKeyword(keyword),
                   Internal("mock"),
                   Range.zero,
                 ),
               ]),
               "should throw ReservedKeyword error",
               keyword,
             )
           )
    ),
  ];
