open Core;

let rec generate = printer =>
  fun
  | A_Statements(stmts) => stmts |> List.iter(unwrap % generate(printer))
  | A_Import(name, imports) => Import.generate(printer, name, imports)
  | A_Declaration(decl) => Declaration.generate(printer, decl);