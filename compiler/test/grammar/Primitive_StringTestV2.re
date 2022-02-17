open Kore;

module Primitive = Grammar.PrimitiveV2;
module U = Util.RawUtilV2;

module Assert =
  Assert.Make({
    type t = N.t(AR.primitive_t, TR.t);

    let parser = _ => Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              AR.typed_node_to_entity("Primitive") % A.Dump.Entity.pp(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive | StringV2"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse"
    >: (
      () =>
        Assert.parse_all(
          AR.of_string("foo") |> U.as_string,
          ["\"foo\"", " \"foo\" "],
        )
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          AR.of_string("foo\\\"bar") |> U.as_string,
          "\"foo\\\"bar\"",
        )
    ),
  ];
