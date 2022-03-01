open Kore;

let check_unary_operation: (A.unary_t, T.t) => option(T.error_t) =
  op =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(valid_type) as type_ =>
      switch (op, valid_type) {
      | (Negative | Positive, `Integer | `Float) => None
      | (Not, `Boolean) => None

      | _ => Some(InvalidUnaryOperation(op, type_))
      };

let check_binary_operation: (A.binary_t, (T.t, T.t)) => option(T.error_t) =
  op =>
    fun
    /* assume they have been reported already and ignore */
    | (Invalid(_), _)
    | (_, Invalid(_)) => None

    | (Valid(valid_lhs) as lhs, Valid(valid_rhs) as rhs) =>
      switch (op, valid_lhs, valid_rhs) {
      | (LogicalAnd | LogicalOr, `Boolean, `Boolean) => None

      | (
          LessOrEqual | LessThan | GreaterOrEqual | GreaterThan | Add | Subtract |
          Divide |
          Multiply |
          Exponent,
          `Integer | `Float,
          `Integer | `Float,
        ) =>
        None

      | (Equal | Unequal, _, _) when valid_lhs == valid_rhs => None

      | _ => Some(InvalidBinaryOperation(op, lhs, rhs))
      };

let check_jsx_class_expression: T.t => option(T.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Boolean) => None

  | type_ => Some(InvalidJSXClassExpression(type_));

let check_jsx_inline_expression: T.t => option(T.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) => None

  | type_ => Some(InvalidJSXInlineExpression(type_));

let rec eval_type_expression: A.TypeExpression.raw_t => T.t =
  type_expr =>
    A.TypeExpression.(
      switch (type_expr) {
      | Nil => Valid(`Nil)
      | Boolean => Valid(`Boolean)
      | Integer => Valid(`Integer)
      | Float => Valid(`Float)
      | String => Valid(`String)
      | Element => Valid(`Element)

      /* use the type of the inner expression to determine type */
      | Group(x) => x |> NR.get_value |> eval_type_expression

      /* use the type of the inner expression to determine type of list items */
      | List(x) => Valid(`List(x |> NR.get_value |> eval_type_expression))

      | Struct(xs) =>
        Valid(
          `Struct(
            xs
            |> List.map(
                 Tuple.map_each2(
                   NR.get_value,
                   NR.get_value % eval_type_expression,
                 ),
               ),
          ),
        )
      | Function(args, res) =>
        Valid(
          `Function((
            args |> List.map(NR.get_value % eval_type_expression),
            res |> NR.get_value |> eval_type_expression,
          )),
        )
      }
    );
