open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.Parser.parse_primitive;
  });

let suite =
  "Grammar.Primitive | Boolean"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse true"
    >: (
      () =>
        Assert.parse_all(
          true |> Primitive.of_boolean |> U.as_node,
          ["true", " true "],
        )
    ),
    "parse false"
    >: (
      () =>
        Assert.parse_all(
          false |> Primitive.of_boolean |> U.as_node,
          ["false", " false "],
        )
    ),
  ];
