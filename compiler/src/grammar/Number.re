open Kore;

let integer =
  Type.K_Integer
  <@ (many1(M.digit) >|= Char.join)
  >== Int64.of_string
  >== AST.of_int
  |> M.lexeme;

let float =
  M.binary_op(
    many1(M.digit) >|= Char.join,
    Character.period,
    many1(M.digit) >|= Char.join,
  )
  >|= (
    ((x, y)) =>
      Block.join(
        ~type_=Type.K_Float,
        ~combine=
          (integer, fraction) => {
            let integer = integer |> String.drop_all_prefix("0");
            let integer_precision = integer |> String.length;
            let fraction = fraction |> String.drop_all_suffix("0");
            let fraction_precision = fraction |> String.length;

            if (fraction == "") {
              (integer |> Float.of_string, integer_precision);
            } else {
              (
                Print.fmt("%s.%s", integer, fraction) |> Float.of_string,
                integer_precision + fraction_precision,
              );
            };
          },
        x,
        y,
      )
  )
  >== AST.of_float
  |> M.lexeme;

let parser = choice([float, integer]);
