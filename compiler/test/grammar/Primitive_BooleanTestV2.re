open Kore;

module Primitive = Grammar.PrimitiveV2;
module U = Util.RawUtilV2;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.parser;
  });

let suite =
  "Grammar.Primitive | BooleanV2"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse true"
    >: (
      () =>
        Assert.parse_all(true |> AR.of_bool |> U.as_bool, ["true", " true "])
    ),
    "parse false"
    >: (
      () =>
        Assert.parse_all(
          false |> AR.of_bool |> U.as_bool,
          ["false", " false "],
        )
    ),
  ];
