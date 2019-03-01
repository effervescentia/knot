open Core;
open Scope;

let analyze_stmt = scope =>
  fun
  | Import(module_, imports) =>
    List.iter(Resolver.of_import(module_) % scope.resolve, imports)
  | Declaration(decl) => {
      Declaration.analyze(scope, decl);

      Resolver.of_declaration(decl) |> scope.resolve;
    }
  | Main(decl) => {
      Declaration.analyze(scope, decl);

      Resolver.of_declaration(decl) |> scope.resolve;
    };

let rec analyze =
  Knot.AST.(
    scope =>
      fun
      | Module(stmts) as res => {
          List.iter(analyze_stmt(scope), stmts);

          let wrapped = await_ctx(res);

          Resolver.of_module(wrapped) |> scope.resolve;

          wrapped;
        }
  );
