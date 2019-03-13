open Core;
open Scope;

let analyze_stmt = scope =>
  fun
  | Import(module_, imports) =>
    List.iter(Resolver.of_import(module_) % scope.resolve, imports)
  | Declaration(name, decl) => {
      Declaration.analyze(scope, name, fst(decl));

      Resolver.of_declaration(name, decl) |> scope.resolve;
    }
  | Main(name, decl) => {
      Declaration.analyze(scope, name, fst(decl));

      Resolver.of_declaration(name, decl) |> scope.resolve;
    };

let analyze =
  Knot.AST.(
    scope =>
      fun
      | Module(stmts) as res => {
          List.iter(analyze_stmt(scope), stmts);

          let wrapped = no_ctx(res);

          Resolver.of_module(wrapped) |> scope.resolve;

          wrapped;
        }
  );
