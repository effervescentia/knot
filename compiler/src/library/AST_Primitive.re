/**
 supported numeric types
  */
type number_t =
  | Integer(Int64.t)
  | Float(float, int);

/**
 a primitive AST node
  */
type primitive_t =
  | Nil
  | Boolean(bool)
  | Number(number_t)
  | String(string);

module Dump = {
  let num_to_string =
    fun
    | Integer(int) => int |> Int64.to_string
    | Float(float, precision) => float |> Fmt.str("%.*f", precision);

  let prim_to_string =
    fun
    | Nil => "Nil"

    | Boolean(bool) => Fmt.str("Boolean(%b)", bool)

    | Number(num) => num |> num_to_string |> Fmt.str("Number(%s)")

    | String(str) => str |> String.escaped |> Fmt.str("String(\"%s\")");
};
