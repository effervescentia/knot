open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;

let _assert_number = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.expression(Target.Common),
      "javascript number matches",
      expected,
      Generator.gen_number(actual),
    )
  );

let suite =
  "Generate.JavaScript_Generator | Number"
  >::: [
    "integer" >: (() => _assert_number(Number("123"), 123L |> of_int)),
    "integer - maximum"
    >: (
      () =>
        _assert_number(
          Number("9223372036854775807"),
          Int64.max_int |> of_int,
        )
    ),
    "integer - minimum"
    >: (
      () =>
        _assert_number(
          Number("-9223372036854775808"),
          Int64.min_int |> of_int,
        )
    ),
    "float - zero"
    >: (() => _assert_number(Number("0"), (0.0, 0) |> of_float)),
    "float - one"
    >: (() => _assert_number(Number("1"), (1.0, 0) |> of_float)),
    "float - many significant digits"
    >: (() => _assert_number(Number("12345"), (12345.0, 5) |> of_float)),
    "float - fraction with many significant decimal digits"
    >: (() => _assert_number(Number("4.56"), (4.56, 3) |> of_float)),
    "float - fraction with significant decimal and non-decimal digits"
    >: (() => _assert_number(Number("1234.56"), (1234.56, 6) |> of_float)),
    "float - very small fraction"
    >: (() => _assert_number(Number("0.000456"), (0.000456, 6) |> of_float)),
    "float - maximum"
    >: (
      () =>
        _assert_number(
          Number("1.79769313486e+308"),
          (Float.max_float, 12) |> of_float,
        )
    ),
    "float - minimum"
    >: (
      () =>
        _assert_number(
          Number("2.22507385851e-308"),
          (Float.min_float, 12) |> of_float,
        )
    ),
  ];
