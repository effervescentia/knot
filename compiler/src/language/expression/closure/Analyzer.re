open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let analyze_closure:
  (
    Scope.t,
    (Scope.t, AST.Raw.statement_t) => AST.Result.statement_t,
    list(AST.Raw.statement_t),
    Range.t
  ) =>
  (list(AST.Result.statement_t), Type.t) =
  (scope, analyze_statement, stmts, range) => {
    let closure_scope = scope |> Scope.create_child(range);
    let stmts' = stmts |> List.map(analyze_statement(closure_scope));

    (
      stmts',
      List.last(stmts') |> Option.map(Node.get_type) |?: Valid(`Nil),
    );
  };
