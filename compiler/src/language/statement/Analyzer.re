open Kore;

let analyze:
  (
    AST.Scope.t,
    (AST.Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    AST.Raw.statement_t
  ) =>
  AST.Result.statement_t =
  (scope, analyze_expression, statement) => {
    let bind = analyze =>
      analyze(scope, analyze_expression)
      % (
        ((value, type_)) =>
          Node.typed(value, type_, Node.get_range(statement))
      );

    statement
    |> fst
    |> Util.fold(
         ~variable=bind(KVariable.analyze),
         ~effect=bind(KEffect.analyze),
       );
  };
