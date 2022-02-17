open Kore;

module Primitive = Grammar.PrimitiveV2;
module U = Util.RawUtilV2;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.boolean;
  });

let suite =
  "Grammar.Primitive | BooleanV2"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse true"
    >: (
      () =>
        Assert.parse_all(AR.of_bool(true) |> U.as_bool, ["true", " true "])
    ),
    "parse false"
    >: (
      () =>
        Assert.parse_all(
          AR.of_bool(false) |> U.as_bool,
          ["false", " false "],
        )
    ),
  ];
