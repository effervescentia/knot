open Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.statement_t
  ) =>
  Result.statement_t =
  (scope, analyze_expression, node) =>
    (
      switch (node) {
      | (Variable(id, expr), _) =>
        KVariable.analyze(scope, analyze_expression, (id, expr))
      | (Expression(expr), _) =>
        KEffect.analyze(scope, analyze_expression, expr)
      }
    )
    |> (((value, type_)) => Node.typed(value, type_, Node.get_range(node)));
