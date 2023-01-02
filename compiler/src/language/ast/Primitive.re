/**
 a primitive AST node
  */
type primitive_t =
  | Nil
  | Boolean(bool)
  | Integer(Int64.t)
  | Float(float, int)
  | String(string);
