open Knot.Kore;
open AST;

let validate: (Operator.Unary.t, Type.t) => option(Type.error_t) =
  operator =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(type_') as type_ =>
      switch (operator, type_') {
      | (Negative | Positive, Integer | Float) => None

      | (Not, Boolean) => None

      | _ => Some(InvalidUnaryOperation(operator, type_))
      };
