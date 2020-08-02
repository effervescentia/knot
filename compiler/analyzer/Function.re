open Globals;

let analyze_scoped_expr = (analyze_expr, scope, expr) => {
  fst(expr)
  |> (
    fun
    | ExpressionStatement(x) => analyze_expr(scope, x)
    | VariableDeclaration(name, x) => analyze_expr(scope, x)
    | VariableAssignment(refr, x) => {
        Reference.analyze(analyze_expr, scope, refr);
        analyze_expr(scope, x);
      }
  );

  Resolver.of_scoped_expression(expr) |> scope.resolve;
};

let analyze = (analyze_expr, scope, params, exprs) => {
  List.iter(Property.analyze_param(analyze_expr, scope), params);
  List.iter(analyze_scoped_expr(analyze_expr, scope), exprs);
};
