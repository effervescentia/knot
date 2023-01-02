module Binary = {
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
    | Exponent;

  let to_string =
    fun
    | LogicalAnd => "LogicalAnd"
    | LogicalOr => "LogicalOr"
    | Add => "Add"
    | Subtract => "Subtract"
    | Divide => "Divide"
    | Multiply => "Multiply"
    | LessOrEqual => "LessOrEqual"
    | LessThan => "LessThan"
    | GreaterOrEqual => "GreaterOrEqual"
    | GreaterThan => "GreaterThan"
    | Equal => "Equal"
    | Unequal => "Unequal"
    | Exponent => "Exponent";
};

module Unary = {
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
