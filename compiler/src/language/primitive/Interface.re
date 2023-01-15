open Knot.Kore;

type t =
  | Nil
  | Boolean(bool)
  | Integer(Int64.t)
  | Float(float, int)
  | String(string);

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = unit;
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) = unit;
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = t;
  });

/* static */

let nil = Nil;
let of_boolean = value => Boolean(value);
let of_integer = value => Integer(value);
let of_float = ((value, precision)) => Float(value, precision);
let of_string = value => String(value);

/* methods */

let fold = (~nil, ~boolean, ~integer, ~float, ~string) =>
  fun
  | Nil => () |> nil
  | Boolean(value) => value |> boolean
  | Integer(value) => value |> integer
  | Float(value, precision) => (value, precision) |> float
  | String(value) => value |> string;
