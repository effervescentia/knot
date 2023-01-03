/**
 a primitive AST node
  */
type t =
  | Nil
  | Boolean(bool)
  | Integer(Int64.t)
  | Float(float, int)
  | String(string);
