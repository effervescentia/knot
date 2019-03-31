open Core;

let generate = (printer, module_name, imports) => {
  imports
  |> List.fold_left(
       acc =>
         fst
         % (
           fun
           | ModuleExport(export_name) =>
             Printf.sprintf("* as %s", export_name)
           | _ => acc
         ),
       "",
     )
  |> (
    s =>
      s == "" ? s : Printf.sprintf("import %s from \"%s\";", s, module_name)
  )
  |> printer;

  gen_list(
    fst
    % (
      fun
      | MainExport(export_name) =>
        Printf.sprintf("%s as %s", main_export, export_name)
      | NamedExport(export_name, original_name) =>
        Printf.sprintf(
          "%s%s",
          export_name,
          switch (original_name) {
          | Some(s) => Printf.sprintf(" as %s", s)
          | None => ""
          },
        )
      | _ => ""
    ),
    imports,
  )
  |> (
    s =>
      s == "" ? s : Printf.sprintf("import {%s} from \"%s\";", s, module_name)
  )
  |> printer;
};
