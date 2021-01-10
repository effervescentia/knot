open Kore;

module Primitive = Grammar.Primitive;

module Target = {
  type t = AST.number_t;

  let parser = Parser.parse(Primitive.integer);

  let test =
    Alcotest.(
      check(
        testable(x => fmt_num % Format.print_string, (==)),
        "program matches",
      )
    );
};

module AssertInteger =
  Assert.Make({
    include Target;

    let parser = Parser.parse(Primitive.integer);
  });
module AssertFloat =
  Assert.Make({
    include Target;

    let parser = Parser.parse(Primitive.float);
  });

let suite =
  "Grammar - Primitive - Number"
  >::: [
    "no parse"
    >: (
      () => {
        ["gibberish"] |> AssertInteger.no_parse;
        ["gibberish"] |> AssertFloat.no_parse;
      }
    ),
    "parse integer"
    >: (
      () =>
        ["123", " 123 "]
        |> AssertInteger.parse_all(Int64.of_int(123) |> AST.of_int)
    ),
    "max integer"
    >: (
      () =>
        AssertInteger.parse(
          "9223372036854775807",
          Int64.max_int |> AST.of_int,
        )
    ),
    "parse float"
    >: (
      () =>
        ["123.45", " 123.45 "]
        |> AssertFloat.parse_all(123.45 |> AST.of_float)
    ),
    "max float"
    >: (
      () =>
        AssertFloat.parse(
          "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000",
          Float.max_float |> AST.of_float,
        )
    ),
  ];
