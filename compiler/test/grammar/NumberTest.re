open Kore;
open Util.RawUtil;

module Number = Grammar.Number;

module Assert =
  Assert.Make({
    open AST;

    type t = Node.Raw.t(number_t);

    let parser = _ => Parser.parse(Number.parser);

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Node.Raw.get_value
              % Raw.Dump.num_to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Number"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse integer"
    >: (
      () =>
        ["123", " 123 "]
        |> Assert.parse_all(123L |> AST.of_int |> as_raw_node)
    ),
    "max integer"
    >: (
      () =>
        Assert.parse(
          "9223372036854775807",
          Int64.max_int |> AST.of_int |> as_raw_node,
        )
    ),
    "parse float"
    >: (
      () =>
        ["123.45", " 123.45 "]
        |> Assert.parse_all((123.45, 5) |> AST.of_float |> as_raw_node)
    ),
    "max float"
    >: (
      () =>
        Assert.parse(
          "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000",
          (Float.max_float, 309) |> AST.of_float |> as_raw_node,
        )
    ),
  ];
