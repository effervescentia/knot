open Kore;

let integer: number_parser_t =
  many1(M.digit)
  >|= Input.join
  >|= N.map(Int64.of_string % AR.of_int)
  >|= N.add_type(TR.(`Integer))
  |> M.lexeme;

let float: number_parser_t =
  M.binary_op(
    many1(M.digit) >|= Input.join,
    Character.period,
    many1(M.digit) >|= Input.join,
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
        |> AR.of_float;

      N.typed(components, TR.(`Float), N.join_ranges(x, y));
    }
  )
  |> M.lexeme;

let parser: number_parser_t = choice([float, integer]);
