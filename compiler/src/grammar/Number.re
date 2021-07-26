open Kore;
open AST.Raw;
open Type.Raw;

let integer =
  many1(M.digit)
  >|= Input.join
  >|= (
    block => (
      block |> Node.Raw.value |> Int64.of_string |> AST.of_int,
      Node.Raw.cursor(block),
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
        let integer = x |> Node.Raw.value |> String.drop_all_prefix("0");
        let integer_precision = integer |> String.length;
        let fraction = y |> Node.Raw.value |> String.drop_all_suffix("0");
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
      Cursor.join(Node.Raw.cursor(x), Node.Raw.cursor(y)),
    )
  )
  |> M.lexeme;

let parser = choice([float, integer]);
