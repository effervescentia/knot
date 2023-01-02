open Knot.Kore;
open AST;

let validate: (Operator.Binary.t, (Type.t, Type.t)) => option(Type.error_t) =
  op =>
    fun
    /* assume they have been reported already and ignore */
    | (Invalid(_), _)
    | (_, Invalid(_)) => None

    | (Valid(valid_lhs) as lhs, Valid(valid_rhs) as rhs) =>
      switch (op, valid_lhs, valid_rhs) {
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

      | (Equal | Unequal, _, _) when valid_lhs == valid_rhs => None

      | _ => Some(InvalidBinaryOperation(op, lhs, rhs))
      };
