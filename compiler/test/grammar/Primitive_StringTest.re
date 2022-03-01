open Kore;

module Primitive = Grammar.Primitive;
module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.parser;
  });

let suite =
  "Grammar.Primitive | String"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse"
    >: (
      () =>
        Assert.parse_all(
          "foo" |> AR.of_string |> U.as_string,
          ["\"foo\"", " \"foo\" "],
        )
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "foo\\\"bar" |> AR.of_string |> U.as_string,
          "\"foo\\\"bar\"",
        )
    ),
  ];
