open Kore;

module Primitive = Grammar.Primitive;
module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.parser;
  });

let suite =
  "Grammar.Primitive | Boolean"
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
