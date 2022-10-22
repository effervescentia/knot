open Knot.Kore;
open Parse.Onyx;

module Character = Grammar.Character;
module Matchers = Grammar.Matchers;

let integer =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % AST.Raw.of_int)
  >|= Node.add_type(Type.Raw.(`Integer))
  |> Matchers.lexeme;

let float =
  Matchers.binary_op(
    many1(Matchers.digit) >|= Input.join,
    Character.period,
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

      Node.typed(components, Type.Raw.(`Float), Node.join_ranges(x, y));
    }
  )
  |> Matchers.lexeme;
