open Core;

let generate_stmt = printer =>
  fun
  | Import(name, imports) => Import.generate(printer, name, imports)
  | Declaration(name, decl)
  | Main(name, decl) => Declaration.generate(printer, name, decl);

let generate = printer =>
  fun
  | Module(stmts) => stmts |> List.iter(generate_stmt(printer));
