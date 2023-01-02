open Kore;
open AST;

let rec analyze: (Scope.t, Raw.expression_t) => Result.expression_t =
  (scope, node) => {
    let node_range = Node.get_range(node);

    (
      switch (node) {
      | (Primitive(prim), _) => (
          Expression.Primitive(prim),
          KPrimitive.analyze(prim),
        )

      | (Identifier(id), _) =>
        let type_ = KIdentifier.analyze(scope, id, node_range);

        (Expression.Identifier(id), type_);

      | (UnaryOp(op, expr), _) =>
        let (expr', type_) =
          KUnaryOperator.analyze(scope, analyze, (op, expr), node_range);

        (Expression.UnaryOp(op, expr'), type_);

      | (BinaryOp(op, lhs, rhs), _) =>
        let (lhs', rhs', type_) =
          KBinaryOperator.analyze(
            scope,
            analyze,
            (op, lhs, rhs),
            node_range,
          );

        (Expression.BinaryOp(op, lhs', rhs'), type_);

      | (Group(expr), _) =>
        let expr' = expr |> KGroup.analyze(scope, analyze);

        (Expression.Group(expr'), Node.get_type(expr'));

      | (Closure(stmts), _) =>
        let (stmts', type_) =
          KClosure.analyze(
            scope,
            s => KStatement.Plugin.analyze(s, analyze),
            stmts,
            node_range,
          );

        (Expression.Closure(stmts'), type_);

      | (DotAccess(expr, prop), _) =>
        let (expr', type_) =
          KDotAccess.analyze(scope, analyze, (expr, prop), node_range);

        (Expression.DotAccess(expr', prop), type_);

      | (BindStyle(source, lhs, rhs), _) =>
        let (source', lhs', rhs') =
          KBindStyle.analyze(scope, analyze, (source, lhs, rhs), node_range);
        let lhs_type = Node.get_type(lhs');

        (Expression.BindStyle(source', lhs', rhs'), lhs_type);

      | (FunctionCall(expr, args), _) =>
        let (expr', args', type_) =
          KFunctionCall.analyze(scope, analyze, (expr, args), node_range);

        (Expression.FunctionCall(expr', args'), type_);

      | (Style(rules), _) =>
        let (rules', type_) =
          KStyle.analyze(scope, analyze, rules, node_range);

        (Expression.Style(rules'), type_);

      | (JSX(jsx), _) =>
        let (jsx', type_) = KSX.analyze(scope, analyze, jsx);

        (Expression.JSX(jsx'), type_);
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, node_range));
  };
