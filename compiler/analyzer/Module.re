open Core;
open Scope;

let analyze_stmt = scope =>
  fun
  | Import(module_, imports) => {
      let wrapped = List.map(await_ctx, imports);
      List.iter(Resolver.of_import % scope.resolve, wrapped);

      A_Import(module_, wrapped);
    }
  | Declaration(decl)
  | Main(decl) => {
      let wrapped = Declaration.analyze(scope, decl) |> await_ctx;
      Resolver.of_declaration(wrapped) |> scope.resolve;

      A_Declaration(wrapped);
    };

let rec analyze =
  Knot.AST.(
    scope =>
      fun
      | Module(stmts) => {
          let wrapped = List.map(analyze_stmt(scope) % await_ctx, stmts);

          A_Module(wrapped);
        }
  );
