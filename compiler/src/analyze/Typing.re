open Kore;

module S = Set.Make(Stdlib.String);

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

  | (expr_type, args) => Some(InvalidFunctionCall(expr_type, args));

let check_jsx_render:
  ((string, T.t, list((string, A.untyped_t(T.t))))) =>
  list((T.error_t, option(Range.t))) =
  fun
  /* assume this have been reported already and ignore */
  | (_, Invalid(_), _) => []

  | (id, Valid(`View(attrs, _)), actual_attrs) => {
      let keys =
        attrs
        @ (actual_attrs |> List.map(Tuple.map_snd2(fst)))
        |> List.map(fst)
        |> List.uniq_by((==));

      let (invalid, missing) =
        keys
        |> List.fold_left(
             ((invalid, missing) as acc, key) => {
               let expected = attrs |> List.assoc_opt(key);
               let actual = actual_attrs |> List.assoc_opt(key);

               switch (expected, actual) {
               | (Some(expected'), Some((actual_value, _) as actual')) =>
                 switch (expected', actual_value) {
                 | (T.Invalid(_), _)
                 | (_, T.Invalid(_)) => acc
                 | (T.Valid(_), T.Valid(_)) when expected' == actual_value => acc
                 | (T.Valid(_), T.Valid(_)) => (
                     invalid
                     @ [
                       (
                         T.InvalidJSXAttribute(key, expected', actual_value),
                         Some(N.get_range(actual')),
                       ),
                     ],
                     missing,
                   )
                 }

               | (Some(expected'), None) => (
                   invalid,
                   missing @ [(key, expected')],
                 )

               | (None, Some(actual')) => (
                   invalid
                   @ [
                     (
                       T.UnexpectedJSXAttribute(key, fst(actual')),
                       Some(N.get_range(actual')),
                     ),
                   ],
                   missing,
                 )

               | (None, None) => acc
               };
             },
             ([], []),
           );

      if (!List.is_empty(invalid)) {
        invalid;
      } else if (!List.is_empty(missing)) {
        [(T.MissingJSXAttributes(id, missing), None)];
      } else {
        [];
      };
    }

  | (id, expr_type, attrs) => [
      (
        InvalidJSXTag(id, expr_type, attrs |> List.map(Tuple.map_snd2(fst))),
        None,
      ),
    ];

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
      | Group((x, _)) => eval_type_expression(x)

      /* use the type of the inner expression to determine type of list items */
      | List((x, _)) => Valid(`List(eval_type_expression(x)))

      | Struct(xs) =>
        Valid(
          `Struct(
            xs |> List.map(Tuple.map_each2(fst, fst % eval_type_expression)),
          ),
        )
      | Function(args, (res, _)) =>
        Valid(
          `Function((
            args |> List.map(fst % eval_type_expression),
            eval_type_expression(res),
          )),
        )
      }
    );
