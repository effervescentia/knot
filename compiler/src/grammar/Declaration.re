open Kore;

let constant = (scope: Scope.t) =>
  Keyword.const
  >> Operator.assign(
       Identifier.parser,
       Expression.parser(scope) >|= AST.of_const,
     )
  >@= (
    (((id, _), decl)) =>
      scope |> Scope.define(id, decl |> TypeOf.declaration)
  )
  |> M.terminated;

let parser = (scope: Scope.t) => choice([constant(scope)]) >|= AST.of_decl;
