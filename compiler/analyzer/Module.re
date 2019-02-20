open Core;
open Scope;

let rec analyze = scope =>
  fun
  | Statements(stmts) => {
      let wrapped = List.map(analyze(scope) % await_ctx, stmts);
      List.iter(Resolver.of_module % scope.resolve, wrapped);

      A_Statements(wrapped);
    }
  | Import(module_, imports) => {
      let wrapped = List.map(await_ctx, imports);
      List.iter(Resolver.of_import % scope.resolve, wrapped);

      A_Import(module_, wrapped);
    }
  | Declaration(decl) => {
      let wrapped = Declaration.analyze(scope, decl) |> await_ctx;
      Resolver.of_declaration(wrapped) |> scope.resolve;

      A_Declaration(wrapped);
    };