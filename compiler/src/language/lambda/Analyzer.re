open Knot.Kore;
open AST;

let analyze_parameter =
    (
      analyze_expression:
        Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr),
      scope: Scope.t('ast),
    ) =>
  Node.analyzer(parameter => {
    let (parameter', type_) =
      switch (fst(parameter)) {
      | (name, None, None) =>
        Type.UntypedFunctionArgument(fst(name))
        |> Scope.report_type_err(scope, Node.get_range(parameter));

        ((name, None, None), Type.Invalid(NotInferrable));

      | (name, None, Some(expression)) =>
        let (expression', expression_type) =
          expression |> Node.analyzer(analyze_expression(scope));

        ((name, None, Some(expression')), expression_type);

      | (name, Some(type_expression), None) =>
        let type_ =
          type_expression
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        ((name, Some(type_expression), None), type_);

      | (name, Some(type_expression), Some(expression)) =>
        let (expression', expression_type) =
          expression |> Node.analyzer(analyze_expression(scope));
        let type_ =
          type_expression
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        switch (expression_type, type_) {
        | (Valid(_), Valid(_)) when expression_type != type_ =>
          Type.TypeMismatch(type_, expression_type)
          |> Scope.report_type_err(scope, Node.get_range(expression))

        /* ignore cases where either type is invalid or when types are equal */
        | _ => ()
        };

        ((name, Some(type_expression), Some(expression')), type_);
      };

    (parameter', type_);
  });

let analyze_parameter_list:
  (
    Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr),
    Scope.t('ast),
    list(Interface.Parameter.node_t('raw_expr, unit))
  ) =>
  list(Interface.Parameter.node_t('result_expr, Type.t)) =
  (analyze_expression, scope, parameters) => {
    let parameters' =
      parameters
      |> List.map(analyze_parameter(analyze_expression, scope) % fst);

    Validator.validate_default_arguments(scope, parameters');

    parameters';
  };
