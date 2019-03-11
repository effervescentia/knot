open Core;

let generate = (printer, module_name, imports) =>
  imports
  |> List.fold_left(
       acc =>
         abandon_ctx
         % (
           fun
           | MainExport(export_name) =>
             acc ++ Printf.sprintf("main as %s", export_name)
           | ModuleExport(export_name) =>
             Printf.sprintf("* as %s", export_name) ++ acc
           | NamedExport(export_name, original_name) =>
             Printf.sprintf(
               "%s%s",
               export_name,
               switch (original_name) {
               | Some(s) => Printf.sprintf(" as %s", s)
               | None => ""
               },
             )
         ),
       "{",
     )
  |> Printf.sprintf("import %s};")
  |> printer;

/* let generate = (printer, module_name, imports) =>
   imports
   |> List.iter(
        abandon_ctx
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
      ); */
