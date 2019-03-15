open Core;
open NestedHashtbl;

let resolve = (module_tbl, symbol_tbl, module_, (value, promise)) =>
  (
    switch (value) {
    | ModuleExport(name) =>
      switch (Hashtbl.find(module_tbl, module_)) {
      | Loaded(_, (_, ast)) =>
        let export_tbl =
          switch (ast^ ^) {
          | Resolved(Module_t(_, x, _)) => x
          | _ => raise(InvalidTypeReference)
          };

        (
          try (Hashtbl.find(export_tbl, name)) {
          | Not_found => raise(InvalidTypeReference)
          }
        )
        |> (
          typ =>
            /* symbol_tbl.add(name, typ); */
            Some(typ)
        );
      | NotLoaded(_) =>
        /* symbol_tbl.add(name, Any_t); */

        Some(resolved(any))
      | exception Not_found =>
        /* Hashtbl.add(module_tbl, module_, NotLoaded([])); */
        /* symbol_tbl.add(name, Any_t); */

        Some(resolved(any))
      }
    | MainExport(name) =>
      /* symbol_tbl.add(name, Any_t); */

      Some(resolved(any))
    | NamedExport(name, alias) =>
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (
        s =>
          /* symbol_tbl.add(s, Any_t); */
          Some(resolved(any))
      )
    }
  )
  |::> promise;
