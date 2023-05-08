open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AST.Common.identifier_t;

    let parser =
      KIdentifier.Plugin.parse_raw % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Dump.identifier_to_xml("Identifier") % Fmt.xml_string(ppf),
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
             Assert.parse_throws_compiler_errs(
               [
                 ParseError(
                   ReservedKeyword(keyword),
                   Internal("mock"),
                   Range.zero,
                 ),
               ],
               keyword,
             )
           )
    ),
  ];