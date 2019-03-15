open Core;
open NestedHashtbl;

let resolve = (module_tbl, symbol_tbl, module_, promise) =>
  (
    switch (fst(promise)) {
    | ModuleExport(name) =>
      switch (Hashtbl.find(module_tbl, module_)) {
      | Loaded(_, ast) =>
        let export_tbl =
          Util.typeof(ast)
          |> (
            fun
            | Some(Module_t(_, x, _)) => x
            | _ => raise(InvalidTypeReference)
          );

        (
          try (Hashtbl.find(export_tbl, name)) {
          | Not_found => raise(InvalidTypeReference)
          }
        )
        |> (
          typ => {
            symbol_tbl.add(name, typ);

            Some(ref(Resolved(typ)));
          }
        );
      | NotLoaded(_) =>
        symbol_tbl.add(name, Any_t);

        Some(ref(Resolved(Any_t)));
      | exception Not_found =>
        Hashtbl.add(module_tbl, module_, NotLoaded([]));
        symbol_tbl.add(name, Any_t);

        Some(ref(Resolved(Any_t)));
      }
    | MainExport(name) =>
      symbol_tbl.add(name, Any_t);

      Some(ref(Resolved(Any_t)));
    | NamedExport(name, alias) =>
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (
        s => {
          symbol_tbl.add(s, Any_t);

          Some(ref(Resolved(Any_t)));
        }
      )
    }
  )
  |::> snd(promise);
