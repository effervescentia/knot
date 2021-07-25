open Kore;
open AST.Raw;
open Type.Raw;

let integer =
  many1(M.digit)
  >|= Input.join
  >|= (
    block => (
      block |> Block.value |> Int64.of_string |> AST.of_int,
      Strong(`Integer),
      Block.cursor(block),
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
        let fraction_precision = String.length(fraction);

        (
          if (fraction == "") {
            (Float.of_string(integer), integer_precision);
          } else {
            (
              Print.fmt("%s.%s", integer, fraction) |> Float.of_string,
              integer_precision + fraction_precision,
            );
          }
        )
        |> of_float;
      },
      Strong(`Float),
      Cursor.join(Block.cursor(x), Block.cursor(y)),
    )
  )
  |> M.lexeme;

let parser = choice([float, integer]);
