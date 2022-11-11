module Binary = {
  /**
 supported binary operators
 */
  type t =
    /* logical operators */
    | LogicalAnd
    | LogicalOr
    /* comparative operators */
    | LessOrEqual
    | LessThan
    | GreaterOrEqual
    | GreaterThan
    /* equality operators */
    | Equal
    | Unequal
    /* arithmetic operators */
    | Add
    | Subtract
    | Divide
    | Multiply
    | Exponent
    | BindStyle;

  let to_string =
    fun
    | LogicalAnd => "And"
    | LogicalOr => "Or"
    | Add => "Add"
    | Subtract => "Sub"
    | Divide => "Div"
    | Multiply => "Mult"
    | LessOrEqual => "LessOrEq"
    | LessThan => "Less"
    | GreaterOrEqual => "GreaterOrEq"
    | GreaterThan => "Greater"
    | Equal => "Equal"
    | Unequal => "Unequal"
    | Exponent => "Exponent"
    | BindStyle => "BindStyle";
};

module Unary = {
  /**
 supported unary operators
 */
  type t =
    | Not
    | Positive
    | Negative;

  let to_string =
    fun
    | Not => "Not"
    | Positive => "Positive"
    | Negative => "Negative";
};
