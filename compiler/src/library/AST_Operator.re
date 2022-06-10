/**
   supported binary operators
   */
type binary_t =
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

/**
   supported unary operators
   */
type unary_t =
  | Not
  | Positive
  | Negative;

module Dump = {
  let binary_to_string = op =>
    switch (op) {
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
    };

  let unary_to_string = op =>
    switch (op) {
    | Not => "Not"
    | Positive => "Positive"
    | Negative => "Negative"
    };
};
