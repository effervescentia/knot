open Knot.Kore;
open AST;

let analyze_closure:
  (
    Scope.t,
    (Scope.t, Raw.statement_t) => Result.statement_t,
    list(Raw.statement_t),
    Range.t
  ) =>
  (list(Result.statement_t), Type.t) =
  (scope, analyze_statement, stmts, range) => {
    let closure_scope = scope |> Scope.create_child(range);
    let stmts' = stmts |> List.map(analyze_statement(closure_scope));

    (
      stmts',
      List.last(stmts') |> Option.map(Node.get_type) |?: Valid(`Nil),
    );
  };
