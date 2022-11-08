open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let float: number_parser_t =
  Matchers.binary_op(
    many1(Matchers.digit) >|= Input.join,
    Matchers.period,
    many1(Matchers.digit) >|= Input.join,
  )
  >|= (
    ((x, y)) => {
      let integer = x |> fst |> String.drop_all_prefix("0");
      let integer_precision = integer |> String.length;
      let fraction = y |> fst |> String.drop_all_suffix("0");
      let fraction_precision = String.length(fraction);

      let components =
        (
          if (fraction == "") {
            (Float.of_string(integer), integer_precision);
          } else {
            (
              Fmt.str("%s.%s", integer, fraction) |> Float.of_string,
              integer_precision + fraction_precision,
            );
          }
        )
        |> AST.Raw.of_float;

      Node.typed(components, (), Node.join_ranges(x, y));
    }
  )
  |> Matchers.lexeme;
