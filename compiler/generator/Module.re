open Core;

let generate_stmt = printer =>
  fun
  | Import(name, imports) => Import.generate(printer, name, imports)
  | Declaration(decl)
  | Main(decl) => Declaration.generate(printer, decl);

let generate = printer =>
  fun
  | Module(stmts) => stmts |> List.iter(generate_stmt(printer));
