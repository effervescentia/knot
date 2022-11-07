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
