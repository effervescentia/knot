open Core;

let generate = (printer, module_name, imports) =>
  imports
  |> List.iter(
       unwrap
       % (
         fun
         | MainExport(export_name) =>
           Printf.sprintf(
             "var %s=%s%s%s",
             export_name,
             module_map,
             Property.gen_access(module_name),
             Property.gen_access(main_export),
           )
         | ModuleExport(export_name) =>
           Printf.sprintf(
             "var %s=%s%s",
             export_name,
             module_map,
             Property.gen_access(module_name),
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