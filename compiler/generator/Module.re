open Core;

let main_export = "__$main_export$";

let gen_import = (printer, module_name, imports) =>
  imports
  |> List.iter(
       (
         fun
         | MainExport(export_name) =>
           Printf.sprintf(
             "var %s=%s%s",
             export_name,
             module_map,
             Property.gen_access(module_name),
           )
         | ModuleExport(export_name) =>
           Printf.sprintf(
             "var %s=%s['%s%s']",
             export_name,
             module_map,
             module_name,
             main_export,
           )
         | NamedExport(export_name, original_name) =>
           Printf.sprintf(
             "var %s=%s%s%s",
             export_name,
             module_map,
             Property.gen_access(module_name),
             (
               switch (original_name) {
               | Some(s) => s
               | None => export_name
               }
             )
             |> Property.gen_access,
           )
       )
       % Printf.sprintf("%s;")
       % printer,
     );

let rec generate = printer =>
  fun
  | Statements(stmts) => stmts |> List.iter(generate(printer))
  | Import(name, imports) => gen_import(printer, name, imports)
  | Declaration(decl) => Declaration.generate(printer, decl);