open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = KPrimitive.Plugin.parse;
  });
let suite =
  "Grammar.Number"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse integer"
    >: (
      () =>
        Assert.parse_all(
          123L |> AR.of_int |> AR.of_num |> U.as_node,
          ["123", " 123 "],
        )
    ),
    "max integer"
    >: (
      () =>
        Assert.parse(
          Int64.max_int |> AR.of_int |> AR.of_num |> U.as_node,
          "9223372036854775807",
        )
    ),
    "parse float"
    >: (
      () =>
        Assert.parse_all(
          (123.45, 5) |> AR.of_float |> AR.of_num |> U.as_node,
          ["123.45", " 123.45 "],
        )
    ),
    "max float"
    >: (
      () =>
        Assert.parse(
          (Float.max_float, 309) |> AR.of_float |> AR.of_num |> U.as_node,
          "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000",
        )
    ),
  ];
