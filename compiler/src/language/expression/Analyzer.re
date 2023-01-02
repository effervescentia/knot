open Kore;
open AST;

let rec analyze: (Scope.t, Raw.expression_t) => Result.expression_t =
  (scope, node) => {
    let node_range = Node.get_range(node);

    (
      switch (node) {
      | (Primitive(primitive), _) => (
          Expression.Primitive(primitive),
          KPrimitive.analyze(primitive),
        )

      | (Identifier(name), _) =>
        let type_ = KIdentifier.analyze(scope, name, node_range);

        (Expression.Identifier(name), type_);

      | (UnaryOp(op, expression), _) =>
        let (expression', type_) =
          KUnaryOperator.analyze(
            scope,
            analyze,
            (op, expression),
            node_range,
          );

        (Expression.UnaryOp(op, expression'), type_);

      | (BinaryOp(op, lhs, rhs), _) =>
        let (lhs', rhs', type_) =
          KBinaryOperator.analyze(
            scope,
            analyze,
            (op, lhs, rhs),
            node_range,
          );

        (Expression.BinaryOp(op, lhs', rhs'), type_);

      | (Group(expression), _) =>
        let expression' = expression |> KGroup.analyze(scope, analyze);

        (Expression.Group(expression'), Node.get_type(expression'));

      | (Closure(statements), _) =>
        let (statements', type_) =
          KClosure.analyze(
            scope,
            s => KStatement.Plugin.analyze(s, analyze),
            statements,
            node_range,
          );

        (Expression.Closure(statements'), type_);

      | (DotAccess(object_, property), _) =>
        let (object_', type_) =
          KDotAccess.analyze(
            scope,
            analyze,
            (object_, property),
            node_range,
          );

        (Expression.DotAccess(object_', property), type_);

      | (BindStyle(kind, view, style), _) =>
        let (kind', view', style') =
          KBindStyle.analyze(
            scope,
            analyze,
            (kind, view, style),
            node_range,
          );
        let lhs_type = Node.get_type(view');

        (Expression.BindStyle(kind', view', style'), lhs_type);

      | (FunctionCall(function_, arguments), _) =>
        let (function_', arguments', type_) =
          KFunctionCall.analyze(
            scope,
            analyze,
            (function_, arguments),
            node_range,
          );

        (Expression.FunctionCall(function_', arguments'), type_);

      | (Style(rules), _) =>
        let (rules', type_) =
          KStyle.analyze(scope, analyze, rules, node_range);

        (Expression.Style(rules'), type_);

      | (KSX(ksx), _) =>
        let (ksx', type_) = KSX.analyze(scope, analyze, ksx);

        (Expression.KSX(ksx'), type_);
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, node_range));
  };
