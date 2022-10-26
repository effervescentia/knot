open Knot.Kore;

let rec validate_default_arguments =
        (~require_default=false, scope: Scope.t, args: list(AST.argument_t)) =>
  switch (args, require_default) {
  | ([], _) => ()

  | ([(AST.{name: (name, _), default: None}, _) as arg, ...xs], true) =>
    Type.DefaultArgumentMissing(name)
    |> Scope.report_type_err(scope, Node.get_range(arg));

    validate_default_arguments(~require_default, scope, xs);

  | ([(AST.{default: Some(_)}, _), ...xs], _) =>
    validate_default_arguments(~require_default=true, scope, xs)

  | ([x, ...xs], _) =>
    validate_default_arguments(~require_default, scope, xs)
  };

let analyze_argument:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.argument_t
  ) =>
  AST.argument_t =
  (scope, analyze_expression, arg) => {
    let (arg', type_) =
      switch (fst(arg)) {
      | {name, default: None, type_: None} =>
        Type.UntypedFunctionArgument(fst(name))
        |> Scope.report_type_err(scope, Node.get_range(arg));

        (
          AST.{name, default: None, type_: None},
          Type.Invalid(NotInferrable),
        );

      | {name, default: Some(expr), type_: None} =>
        let expr' = expr |> analyze_expression(scope);

        (
          AST.{name, default: Some(expr'), type_: None},
          Node.get_type(expr'),
        );

      | {name, default: None, type_: Some(type_expr)} =>
        let type_ =
          type_expr
          |> fst
          |> KTypeExpression.Plugin.analyze(SymbolTable.create());

        (AST.{name, default: None, type_: Some(type_expr)}, type_);

      | {name, default: Some(expr), type_: Some(type_expr)} =>
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

        (AST.{name, default: Some(expr'), type_: Some(type_expr)}, type_);
      };

    Node.typed(arg', type_, Node.get_range(arg));
    /* ignore cases where either type is invalid or when types are equal */
  };

let analyze_argument_list:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    list(AST.Raw.argument_t)
  ) =>
  list(AST.argument_t) =
  (scope, analyze_expression, args) => {
    let args' =
      args |> List.map(analyze_argument(scope, analyze_expression));

    validate_default_arguments(scope, args');

    args';
  };
