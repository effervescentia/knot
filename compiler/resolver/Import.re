open Globals;

let resolve = (module_tbl, symbol_tbl, module_, (value, promise)) => {
  let module_type =
    switch (Hashtbl.find(module_tbl, module_)) {
    | Loaded(ast) => opt_type_ref(ast)

    /* module definition has been injected by the compiler */
    | Injected(type_) => type_

    | Failed
    | Resolving => throw(InvalidImportTarget(module_))

    /* module has not been registered */
    | exception Not_found => invariant(ImportedModuleDoesNotExist(module_))
    };

  let add_symbol = NestedHashtbl.add(symbol_tbl);

  (
    switch (module_type, value) {
    | (Module_t(_, export_tbl, _), ModuleExport(name)) =>
      (
        Hashtbl.length(export_tbl) == 0
          ? throw_semantic(ModuleDoesNotContainExports(module_))
          : Object_t(export_tbl)
      )
      =<< add_symbol(name)

    | (Module_t(_, _, main_export), MainExport(name)) =>
      main_export
      |!> CompilationError(SemanticError(MainExportDoesNotExist(module_)))
      =<< add_symbol(name)

    | (Module_t(_, export_tbl, _), NamedExport(name, alias)) =>
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (
        s =>
          (
            try(Hashtbl.find(export_tbl, name)) {
            | Not_found => throw_semantic(ExportDoesNotExist(module_, name))
            }
          )
          =<< add_symbol(s)
      )

    | _ => invariant(InvalidModuleType(module_))
    }
  )
  <:= promise;
};
