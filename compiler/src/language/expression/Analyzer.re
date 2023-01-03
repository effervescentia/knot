open Kore;

let rec analyze: (AST.Scope.t, AST.Raw.expression_t) => AST.Result.expression_t =
  (scope, node) => {
    let node_range = Node.get_range(node);

    (
      switch (node) {
      | (Primitive(primitive), _) => (
          AST.Expression.Primitive(primitive),
          KPrimitive.analyze(primitive),
        )

      | (Identifier(name), _) =>
        let type_ = KIdentifier.analyze(scope, name, node_range);

        (AST.Result.of_id(name), type_);

      | (UnaryOp(op, expression), _) =>
        let (expression', type_) =
          KUnaryOperator.analyze(
            scope,
            analyze,
            (op, expression),
            node_range,
          );

        (expression' |> AST.Result.of_unary_op(op), type_);

      | (BinaryOp(op, lhs, rhs), _) =>
        let (lhs', rhs', type_) =
          KBinaryOperator.analyze(
            scope,
            analyze,
            (op, lhs, rhs),
            node_range,
          );

        ((lhs', rhs') |> AST.Result.of_binary_op(op), type_);

      | (Group(expression), _) =>
        let expression' = expression |> KGroup.analyze(scope, analyze);

        (AST.Result.of_group(expression'), Node.get_type(expression'));

      | (Closure(statements), _) =>
        let (statements', type_) =
          KClosure.analyze(
            scope,
            s => KStatement.Plugin.analyze(s, analyze),
            statements,
            node_range,
          );

        (AST.Result.of_closure(statements'), type_);

      | (DotAccess(object_, property), _) =>
        let (object_', type_) =
          KDotAccess.analyze(
            scope,
            analyze,
            (object_, property),
            node_range,
          );

        ((object_', property) |> AST.Result.of_dot_access, type_);

      | (BindStyle(kind, view, style), _) =>
        let (kind', view', style') =
          KBindStyle.analyze(
            scope,
            analyze,
            (kind, view, style),
            node_range,
          );
        let lhs_type = Node.get_type(view');

        ((view', style') |> AST.Result.of_bind_style(kind'), lhs_type);

      | (FunctionCall(function_, arguments), _) =>
        let (function_', arguments', type_) =
          KFunctionCall.analyze(
            scope,
            analyze,
            (function_, arguments),
            node_range,
          );

        ((function_', arguments') |> AST.Result.of_func_call, type_);

      | (Style(rules), _) =>
        let (rules', type_) =
          KStyle.analyze(scope, analyze, rules, node_range);

        (AST.Result.of_style(rules'), type_);

      | (KSX(ksx), _) =>
        let (ksx', type_) = KSX.analyze(scope, analyze, ksx);

        (AST.Result.of_ksx(ksx'), type_);
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, node_range));
  };
