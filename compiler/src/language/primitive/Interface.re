open Knot.Kore;

type t =
  | Nil
  | Boolean(bool)
  | Integer(Int64.t)
  | Float(float, int)
  | String(string);

/* helpers */
let nil = Nil;
let boolean = x => Boolean(x);
let integer = x => Integer(x);
let float = ((x, precision)) => Float(x, precision);
let string = x => String(x);
