open Globals;

let generate_import = (printer, core) =>
  fun
  | Import(name, imports) =>
    Import.generate(printer, core, core.to_module_name(name), imports);

let generate_stmt = (printer, core) =>
  fun
  | Declaration(name, decl) =>
    Declaration.generate(printer, core, name, decl)
  | Main(name, decl) => {
      Declaration.generate(printer, core, name, decl);

      core.to_export_statement(name, Some(main_export)) |> printer;
    };

let generate = (printer, core) =>
  fun
  | Module(imports, stmts) => {
      imports |> List.iter(generate_import(printer, core));
      stmts |> List.iter(generate_stmt(printer, core));
    };
