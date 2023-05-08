open Knot.Kore;
open AST;

let validate: (Operator.Binary.t, (Type.t, Type.t)) => option(Type.error_t) =
  operator =>
    fun
    /* assume they have been reported already and ignore */
    | (Invalid(_), _)
    | (_, Invalid(_)) => None

    | (Valid(lhs') as lhs, Valid(rhs') as rhs) =>
      switch (operator, lhs', rhs') {
      | (LogicalAnd | LogicalOr, Boolean, Boolean) => None

      | (
          LessOrEqual | LessThan | GreaterOrEqual | GreaterThan | Add | Subtract |
          Divide |
          Multiply |
          Exponent,
          Integer | Float,
          Integer | Float,
        ) =>
        None

      | (Equal | Unequal, _, _) when lhs' == rhs' => None

      | _ => Some(InvalidBinaryOperation(operator, lhs, rhs))
      };
