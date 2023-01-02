open Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.statement_t
  ) =>
  Result.statement_t =
  (scope, analyze_expression, node) => {
    let (&>) = (args, analyze) =>
      args
      |> analyze(scope, analyze_expression)
      |> (
        ((value, type_)) => Node.typed(value, type_, Node.get_range(node))
      );

    switch (node) {
    | (Variable(id, expr), _) => (id, expr) &> KVariable.analyze
    | (Effect(expr), _) => expr &> KEffect.analyze
    };
  };
