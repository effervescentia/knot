open Core;

let generate = (printer, core, module_name, imports) => {
  let module_import =
    imports
    |> List.fold_left(
         acc =>
           fst
           % (
             fun
             | ModuleExport(export_name) => Some(export_name)
             | _ => acc
           ),
         None,
       );

  let named_imports =
    imports
    |> List.fold_left(
         acc =>
           fst
           % (
             fun
             | MainExport(export_name) => [
                 (main_export, Some(export_name)),
                 ...acc,
               ]
             | NamedExport(original_name, alias_name) => [
                 (original_name, alias_name),
                 ...acc,
               ]
             | _ => acc
           ),
         [],
       );

  switch (module_import, named_imports) {
  | (None, []) => ()
  | _ =>
    core.to_import_statement(module_name, module_import, named_imports)
    |> printer
  };
};
