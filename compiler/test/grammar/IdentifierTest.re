open Kore;

module Identifier = Grammar.Identifier;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.identifier_t;

    let parser = ((_, ctx)) =>
      ctx |> Identifier.parser |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, id) =>
              A.Dump.(
                id
                |> untyped_node_to_entity(
                     ~attributes=[
                       ("value", id |> fst |> Reference.Identifier.to_string),
                     ],
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
    >: (() => Assert.parse("foo" |> AR.of_public |> U.as_untyped, "foo")),
    "parse private identifier"
    >: (() => Assert.parse("foo" |> AR.of_private |> U.as_untyped, "_foo")),
    "throw error on reserved keywords"
    >: (
      () =>
        Constants.Keyword.reserved
        |> List.iter(keyword =>
             Assert.parse_throws(
               CompileError([
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
