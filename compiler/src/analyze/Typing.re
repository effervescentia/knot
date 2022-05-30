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

let check_dot_access: (string, T.t) => option(T.error_t) =
  prop =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(`Struct(props))
        when props |> List.exists(((name, _)) => name == prop) =>
      None

    | type_ => Some(InvalidDotAccess(type_, prop));

let check_function_call: ((T.t, list(T.t))) => option(T.error_t) =
  fun
  /* assume this have been reported already and ignore */
  | (Invalid(_), _) => None

  | (Valid(`Function(args, _)) as func_type, actual_args) =>
    if (List.length(args) != List.length(actual_args)) {
      Some(InvalidFunctionCall(func_type, actual_args));
    } else {
      List.combine(args, actual_args)
      |> List.fold_left(
           (err, args) =>
             Option.(
               switch (err, args) {
               | (Some(_), _) => err

               | (_, (T.Valid(_), T.Valid(_))) when fst(args) == snd(args) =>
                 None

               /* ignore it if the actual arg type is invalid */
               | (_, (T.Valid(_), T.Invalid(_))) => None

               | _ => Some(T.InvalidFunctionCall(func_type, actual_args))
               }
             ),
           None,
         );
    }

  | (expr_type, arg_types) =>
    Some(InvalidFunctionCall(expr_type, arg_types));

let check_jsx_class_expression: T.t => option(T.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Boolean) => None

  | type_ => Some(InvalidJSXClassExpression(type_));

let check_jsx_primitive_expression: T.t => option(T.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) => None

  | type_ => Some(InvalidJSXPrimitiveExpression(type_));

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
