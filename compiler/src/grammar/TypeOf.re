open Kore;
open AST;
open Type;

let _typeof_arithmetic_op = (l, r) =>
  switch (l, r) {
  | (K_Integer, K_Integer) => K_Integer
  | (K_Float, _)
  | (_, K_Float) => K_Float
  | _ => assert(false)
  };

let rec expression =
  fun
  | Primitive(x) => Tuple.snd3(x)
  | Identifier(_) => K_Unknown
  | JSX(_) => K_Element
  | Group(x) => Block.value(x) |> expression
  /* no unary ops that change the type of the expression */
  | UnaryOp(_, x) => expression(x)
  | BinaryOp(op, l, r) =>
    switch (op) {
    | LogicalAnd
    | LogicalOr
    | LessThan
    | LessOrEqual
    | GreaterThan
    | GreaterOrEqual
    | Equal
    | Unequal => K_Boolean

    | Subtract
    | Multiply => _typeof_arithmetic_op(expression(l), expression(r))

    | Divide
    | Exponent =>
      switch (expression(l), expression(r)) {
      | (K_Integer | K_Float, K_Integer | K_Float) => K_Float
      | _ => assert(false)
      }

    | Add =>
      switch (expression(l), expression(r)) {
      | (K_String, K_String) => K_String
      | (l, r) => _typeof_arithmetic_op(l, r)
      }
    }
  | Closure(x) => Block.type_(x);

let statement =
  fun
  | Expression(x) => expression(x)
  | Variable(_) => K_Nil;
