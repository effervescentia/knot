open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.statement_t) => Result.statement_t,
    list(Raw.statement_t),
    Range.t
  ) =>
  (list(Result.statement_t), Type.t) =
  (scope, analyze_statement, statements, range) => {
    let closure_scope = scope |> Scope.create_child(range);
    let statements' =
      statements |> List.map(analyze_statement(closure_scope));

    (
      statements',
      List.last(statements') |> Option.map(Node.get_type) |?: Valid(`Nil),
    );
  };
