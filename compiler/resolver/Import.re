open Core;
open NestedHashtbl;

exception InferredModuleType;

let resolve = (module_tbl, symbol_tbl, module_, (value, promise)) => {
  let module_type =
    switch (Hashtbl.find(module_tbl, module_)) {
    | Loaded(ast) =>
      let ast_ref = opt_type_ref(ast);
      switch (ast_ref^) {
      /* inferred module types are not allowed */
      | Inferred(_) => raise(InferredModuleType)

      /* module has been loaded and linked */
      | _ => Some(ast_ref)
      };

    /* module definition has been injected by the compiler */
    | Injected(type_) => Some(declared(type_))

    /* module has not been registered */
    | exception Not_found =>
      /* Hashtbl.add(module_tbl, module_, NotLoaded([])); */
      /* symbol_tbl.add(name, Any_t); */

      /* declared(any) */
      None
    };

  (
    switch (module_type, value) {
    | (
        Some({contents: Declared(Module_t(_, export_tbl, _))}),
        ModuleExport(name),
      ) =>
      (
        try (Hashtbl.find(export_tbl, name)) {
        | Not_found => raise(InvalidTypeReference)
        }
      )
      =<< symbol_tbl.add(name)

    | (
        Some({contents: Declared(Module_t(_, _, main_export))}),
        MainExport(name),
      ) =>
      (
        switch (main_export) {
        | Some(typ) => typ
        | None => raise(InvalidTypeReference)
        }
      )
      =<< symbol_tbl.add(name)

    | (
        Some({contents: Declared(Module_t(_, export_tbl, _))}),
        NamedExport(name, alias),
      ) =>
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
            | Not_found => inferred(any) =<< Hashtbl.add(export_tbl, name)
            }
          )
          =<< symbol_tbl.add(s)
      )

    | _ => defined(any)
    }
  )
  <:= promise;
};