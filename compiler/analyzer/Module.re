open Core;

let rec analyze = scope =>
  fun
  | Statements(stmts) =>
    A_Statements(List.map(analyze(scope) % wrap, stmts))
  | Import(module_, imports) => {
      List.iter(Import.analyze(scope, module_), imports);

      A_Import(module_, List.map(wrap, imports));
    }
  | Declaration(decl) =>
    A_Declaration(Declaration.analyze(scope, decl) |> wrap);