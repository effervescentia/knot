open Knot.Kore;

type t =
  | Nil
  | Boolean(bool)
  | Integer(Int64.t)
  | Float(float, int)
  | String(string);

/* helpers */
let nil = Nil;
let boolean = boolean => Boolean(boolean);
let integer = integer => Integer(integer);
let float = ((float, precision)) => Float(float, precision);
let string = string => String(string);
