open Core;

let generate = (printer, module_name, imports) =>
  imports
  |> List.fold_left(
       acc =>
         fst
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
  |> (s => Printf.sprintf("import %s} from \"%s\";", s, module_name))
  |> printer;
