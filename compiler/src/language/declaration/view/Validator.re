open Knot.Kore;
open AST;

let validate_jsx_primitive_expression: Type.t => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(Nil | Boolean | Integer | Float | String | Element) => None

  | type_ => Some(InvalidKSXPrimitiveExpression(type_));
