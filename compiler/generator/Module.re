open Core;

let generate_stmt = printer =>
  fun
  | A_Import(name, imports) => Import.generate(printer, name, imports)
  | A_Declaration(decl)
  | A_Main(decl) => Declaration.generate(printer, decl);

let generate = printer =>
  fun
  | A_Module(stmts) =>
    stmts |> List.iter(abandon_ctx % generate_stmt(printer));
