open Knot.Kore;

let rec analyze_expression: (Scope.t, AST.Raw.expression_t) => AST.expression_t =
  (scope, node) => {
    let node_range = Node.get_range(node);

    (
      switch (node) {
      | (Primitive(prim), _) => (
          AST.Primitive(prim),
          KPrimitive.Plugin.analyze(prim),
        )

      | (Identifier(id), _) =>
        let type_ = KIdentifier.Plugin.analyze(scope, id, node_range);

        (AST.Identifier(id), type_);

      | (UnaryOp(op, expr), _) =>
        let (expr', type_) =
          KUnaryOperator.Plugin.analyze(
            scope,
            analyze_expression,
            (op, expr),
            node_range,
          );

        (AST.UnaryOp(op, expr'), type_);

      | (BinaryOp(op, lhs, rhs), _) =>
        let (lhs', rhs', type_) =
          KBinaryOperator.Plugin.analyze(
            scope,
            analyze_expression,
            (op, lhs, rhs),
            node_range,
          );

        (AST.BinaryOp(op, lhs', rhs'), type_);

      | (Group(expr), _) =>
        let expr' = expr |> KGroup.Plugin.analyze(scope, analyze_expression);

        (AST.Group(expr'), Node.get_type(expr'));

      | (Closure(stmts), _) =>
        let (stmts', type_) =
          KClosure.Plugin.analyze(
            scope,
            s => KStatement.Plugin.analyze(s, analyze_expression(s)),
            stmts,
            node_range,
          );

        (AST.Closure(stmts'), type_);

      | (DotAccess(expr, prop), _) =>
        let (expr', type_) =
          KDotAccess.Plugin.analyze(
            scope,
            analyze_expression,
            (expr, prop),
            node_range,
          );

        (AST.DotAccess(expr', prop), type_);

      | (FunctionCall(expr, args), _) =>
        let (expr', args', type_) =
          KFunctionCall.Plugin.analyze(
            scope,
            analyze_expression,
            (expr, args),
            node_range,
          );

        (AST.FunctionCall(expr', args'), type_);

      | (Style(rules), _) =>
        let (rules', type_) =
          KStyle.Plugin.analyze(scope, analyze_expression, rules, node_range);

        (AST.Style(rules'), type_);

      | (JSX(jsx), _) =>
        let (jsx', type_) =
          KSX.Plugin.analyze(scope, analyze_expression, jsx);

        (AST.JSX(jsx'), type_);
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, node_range));
  };
