open Kore;

let integer: number_parser_t =
  many1(M.digit)
  >|= Input.join
  >|= NR.map_value(Int64.of_string % AR.of_int)
  >|= N.of_raw(TR.Valid(`Integer))
  |> M.lexeme;

let float: number_parser_t =
  M.binary_op(
    many1(M.digit) >|= Input.join,
    Character.period,
    many1(M.digit) >|= Input.join,
  )
  >|= (
    ((x, y)) => {
      let integer = x |> NR.get_value |> String.drop_all_prefix("0");
      let integer_precision = integer |> String.length;
      let fraction = y |> NR.get_value |> String.drop_all_suffix("0");
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

      N.create(components, TR.Valid(`Float), NR.join_ranges(x, y));
    }
  )
  |> M.lexeme;

let parser: number_parser_t = choice([float, integer]);
