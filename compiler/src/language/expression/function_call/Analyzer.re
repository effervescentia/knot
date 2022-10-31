open Knot.Kore;

let validate_function_call: ((Type.t, list(Type.t))) => option(Type.error_t) =
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

               | (_, (Type.Valid(_), Type.Valid(_)))
                   when fst(args) == snd(args) =>
                 None

               /* ignore it if the actual arg type is invalid */
               | (_, (Type.Valid(_), Type.Invalid(_))) => None

               | _ => Some(Type.InvalidFunctionCall(func_type, actual_args))
               }
             ),
           None,
         );
    }

  | (expr_type, args) => Some(InvalidFunctionCall(expr_type, args));

let analyze_function_call:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    (AST.Raw.expression_t, list(AST.Raw.expression_t)),
    Range.t
  ) =>
  (AST.expression_t, list(AST.expression_t), Type.t) =
  (scope, analyze_expression, (expr, args), range) => {
    let expr' = analyze_expression(scope, expr);
    let args' = args |> List.map(analyze_expression(scope));
    let type_expr = Node.get_type(expr');
    let type_args = args' |> List.map(Node.get_type);

    (type_expr, type_args)
    |> validate_function_call
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expr',
      args',
      switch (type_expr) {
      | Valid(`Function(_, result)) => result
      | _ => Invalid(NotInferrable)
      },
    );
  };
