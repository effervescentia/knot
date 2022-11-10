open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = KPrimitive.Plugin.parse_primitive;
  });

let suite =
  "Grammar.Primitive | String"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse"
    >: (
      () =>
        Assert.parse_all(
          "foo" |> AR.of_string |> U.as_node,
          ["\"foo\"", " \"foo\" "],
        )
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "foo\\\"bar" |> AR.of_string |> U.as_node,
          "\"foo\\\"bar\"",
        )
    ),
  ];
