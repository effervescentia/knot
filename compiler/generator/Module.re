open Core;

let generate_stmt = (printer, to_module_name) =>
  fun
  | Import(name, imports) =>
    Import.generate(printer, to_module_name(name), imports)
  | Declaration(name, decl) => Declaration.generate(printer, name, decl)
  | Main(name, decl) => {
      Declaration.generate(printer, name, decl);

      Printf.sprintf("export {%s as main};", name) |> printer;
    };

let generate = (printer, to_module_name) =>
  fun
  | Module(stmts) =>
    stmts |> List.iter(generate_stmt(printer, to_module_name));
