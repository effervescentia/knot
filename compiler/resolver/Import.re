open Core;
open NestedHashtbl;

let resolve = (module_tbl, symbol_tbl, module_, (value, promise)) => {
  let module_type =
    switch (Hashtbl.find(module_tbl, module_)) {
    | Loaded(ast) => Some(opt_type_ref(ast))

    /* module definition has been injected by the compiler */
    | Injected(type_) => Some(type_)

    | Failed
    | Resolving => raise(InvalidImport)

    /* module has not been registered */
    | exception Not_found => raise(ImportedModuleDoesNotExist)
    };

  (
    switch (module_type, value) {
    | (Some(Module_t(_, export_tbl, _)), ModuleExport(name)) =>
      (
        try (Hashtbl.find(export_tbl, name)) {
        | Not_found => raise(InvalidTypeReference)
        }
      )
      =<< symbol_tbl.add(name)

    | (Some(Module_t(_, _, main_export)), MainExport(name)) =>
      (
        switch (main_export) {
        | Some(typ) => typ
        | None => raise(InvalidTypeReference)
        }
      )
      =<< symbol_tbl.add(name)

    | (Some(Module_t(_, export_tbl, _)), NamedExport(name, alias)) =>
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (
        s =>
          (
            try (Hashtbl.find(export_tbl, name)) {
            | Not_found => raise(InvalidTypeReference)
            }
          )
          =<< symbol_tbl.add(s)
      )

    | _ => raise(InvalidTypeReference)
    }
  )
  <:= promise;
};
