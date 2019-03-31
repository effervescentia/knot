open Core;

let analyze_scoped_expr = (scope, expr) => {
  fst(expr)
  |> (
    fun
    | ExpressionStatement(x) => Expression.analyze(scope, x)
    | VariableDeclaration(name, x) => Expression.analyze(scope, x)
    | VariableAssignment(refr, x) => {
        Reference.analyze(Expression.analyze, scope, refr);
        Expression.analyze(scope, x);
      }
  );

  Resolver.of_scoped_expression(expr) |> scope.resolve;
};
