open Core;
open NestedHashtbl;

let resolve = (module_tbl, symbol_tbl, module_, (value, promise)) => {
  let module_type =
    switch (Hashtbl.find(module_tbl, module_)) {
    /* module has been loaded and linked */
    | Loaded(_, ast) => Some(opt_type_ref(ast))

    /* module is not loaded but has callbacks registered */
    | NotLoaded(_) => Some(declared(any))

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

    | (_, NamedExport(name, alias)) =>
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (
        s =>
          /* symbol_tbl.add(s, Any_t); */
          declared(any)
      )

    | _ => inferred(any)
    }
  )
  <:= promise;
};
