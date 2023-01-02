open Knot.Kore;
open AST;

let analyze_argument:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.parameter_t
  ) =>
  Result.parameter_t =
  (scope, analyze_expression, arg) => {
    let (arg', type_) =
      switch (fst(arg)) {
      | (name, None, None) =>
        Type.UntypedFunctionArgument(fst(name))
        |> Scope.report_type_err(scope, Node.get_range(arg));

        ((name, None, None), Type.Invalid(NotInferrable));

      | (name, None, Some(expr)) =>
        let expr' = expr |> analyze_expression(scope);

        ((name, None, Some(expr')), Node.get_type(expr'));

      | (name, Some(type_expr), None) =>
        let type_ =
          type_expr
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        ((name, Some(type_expr), None), type_);

      | (name, Some(type_expr), Some(expr)) =>
        let expr' = expr |> analyze_expression(scope);
        let expr_type = Node.get_type(expr');
        let type_ =
          type_expr
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        switch (expr_type, type_) {
        | (Valid(_), Valid(_)) when expr_type != type_ =>
          Type.TypeMismatch(type_, expr_type)
          |> Scope.report_type_err(scope, Node.get_range(expr))

        | _ => ()
        };

        ((name, Some(type_expr), Some(expr')), type_);
      };

    Node.typed(arg', type_, Node.get_range(arg));
    /* ignore cases where either type is invalid or when types are equal */
  };

let analyze_argument_list:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    list(Raw.parameter_t)
  ) =>
  list(Result.parameter_t) =
  (scope, analyze_expression, args) => {
    let args' =
      args |> List.map(analyze_argument(scope, analyze_expression));

    Validator.validate_default_arguments(scope, args');

    args';
  };
