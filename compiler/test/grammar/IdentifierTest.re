open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.identifier_t;

    let parser =
      KIdentifier.Plugin.parse % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, id) =>
              AST.Raw.Dump.(
                id
                |> untyped_node_to_entity(
                     ~attributes=[("value", fst(id))],
                     "Identifier",
                   )
                |> Entity.pp(ppf)
              ),
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
