/**
 a primitive AST node
  */
type t =
  | Nil
  | Boolean(bool)
  | Integer(int64)
  | Float(float, int)
  | String(string);
