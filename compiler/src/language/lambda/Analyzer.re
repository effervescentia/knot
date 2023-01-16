open Knot.Kore;
open AST;

let analyze_parameter:
  (
    Scope.t('ast),
    (Scope.t('ast), Raw.expression_t) => Result.expression_t,
    Raw.parameter_t
  ) =>
  Result.parameter_t =
  (scope, analyze_expression, parameter) => {
    let (parameter', type_) =
      switch (fst(parameter)) {
      | (name, None, None) =>
        Type.UntypedFunctionArgument(fst(name))
        |> Scope.report_type_err(scope, Node.get_range(parameter));

        ((name, None, None), Type.Invalid(NotInferrable));

      | (name, None, Some(expression)) =>
        let expression' = expression |> analyze_expression(scope);

        ((name, None, Some(expression')), Node.get_type(expression'));

      | (name, Some(type_expression), None) =>
        let type_ =
          type_expression
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        ((name, Some(type_expression), None), type_);

      | (name, Some(type_expression), Some(expression)) =>
        let expression' = expression |> analyze_expression(scope);
        let expression_type = Node.get_type(expression');
        let type_ =
          type_expression
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        switch (expression_type, type_) {
        | (Valid(_), Valid(_)) when expression_type != type_ =>
          Type.TypeMismatch(type_, expression_type)
          |> Scope.report_type_err(scope, Node.get_range(expression))

        | _ => ()
        };

        ((name, Some(type_expression), Some(expression')), type_);
      };

    Node.typed(parameter', type_, Node.get_range(parameter));
    /* ignore cases where either type is invalid or when types are equal */
  };

let analyze_parameter_list:
  (
    Scope.t('ast),
    (Scope.t('ast), Node.t('raw_expr, unit)) => Node.t('result_expr, Type.t),
    list(Interface.Parameter.node_t('raw_expr, unit))
  ) =>
  list(Interface.Parameter.node_t('raw_expr, Type.t)) =
  (scope, analyze_expression, parameters) => {
    let parameters' =
      parameters
      |> List.map(
           Node.analyzer(analyze_parameter(scope, analyze_expression)),
         )
      |> List.split;

    Validator.validate_default_arguments(scope, parameters');

    parameters';
  };
