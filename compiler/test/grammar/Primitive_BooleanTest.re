open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = KPrimitive.Plugin.parse;
  });

let suite =
  "Grammar.Primitive | Boolean"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse true"
    >: (
      () =>
        Assert.parse_all(true |> AR.of_bool |> U.as_node, ["true", " true "])
    ),
    "parse false"
    >: (
      () =>
        Assert.parse_all(
          false |> AR.of_bool |> U.as_node,
          ["false", " false "],
        )
    ),
  ];
