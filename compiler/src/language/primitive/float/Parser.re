open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t =
  Matchers.binary_op(
    many1(Matchers.digit) >|= Input.join,
    Matchers.period,
    many1(Matchers.digit) >|= Input.join,
  )
  >|= (
    ((integer, fraction)) => {
      let integer' = integer |> fst |> String.drop_all_prefix("0");
      let integer_precision = integer' |> String.length;
      let fraction' = fraction |> fst |> String.drop_all_suffix("0");
      let fraction_precision = String.length(fraction');

      let components =
        if (fraction' == "") {
          (Float.of_string(integer'), integer_precision);
        } else {
          (
            Fmt.str("%s.%s", integer', fraction') |> Float.of_string,
            integer_precision + fraction_precision,
          );
        };

      Node.raw(components, Node.join_ranges(integer, fraction));
    }
  )
  |> Matchers.lexeme;
