open Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.statement_t
  ) =>
  Result.statement_t =
  (scope, analyze_expression, statement) => {
    let bind = analyze =>
      analyze(scope, analyze_expression)
      % (
        ((value, type_)) =>
          Node.typed(value, type_, Node.get_range(statement))
      );

    statement
    |> fst
    |> Statement.fold(
         ~variable=bind(KVariable.analyze),
         ~effect=bind(KEffect.analyze),
       );
  };
