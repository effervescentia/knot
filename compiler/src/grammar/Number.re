open Kore;

let integer =
  many1(M.digit)
  >|= Input.join
  >|= (
    block => (
      block |> Block.value |> Int64.of_string |> RawUtil.int,
      Type.K_Strong(K_Integer),
      block |> Block.cursor,
    )
  )
  |> M.lexeme;

let float =
  M.binary_op(
    many1(M.digit) >|= Input.join,
    Character.period,
    many1(M.digit) >|= Input.join,
  )
  >|= (
    ((x, y)) => (
      {
        let integer = x |> Block.value |> String.drop_all_prefix("0");
        let integer_precision = integer |> String.length;
        let fraction = y |> Block.value |> String.drop_all_suffix("0");
        let fraction_precision = fraction |> String.length;

        (
          fraction == ""
            ? (integer |> Float.of_string, integer_precision)
            : (
              Print.fmt("%s.%s", integer, fraction) |> Float.of_string,
              integer_precision + fraction_precision,
            )
        )
        |> RawUtil.float;
      },
      Type.K_Strong(K_Float),
      Cursor.join(x |> Block.cursor, y |> Block.cursor),
    )
  )
  |> M.lexeme;

let parser = choice([float, integer]);
