open Core;
open NestedHashtbl;

let resolve = (module_tbl, symbol_tbl, module_, promise) =>
  fun
  | ModuleExport(name) => {
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
        |> symbol_tbl.add(name);
      | NotLoaded(_) => symbol_tbl.add(name, Util.generate_any_type())
      | exception Not_found =>
        Hashtbl.add(module_tbl, module_, NotLoaded([]));
        symbol_tbl.add(name, Util.generate_any_type());
      };

      None;
    }
  | MainExport(name) => {
      symbol_tbl.add(name, Util.generate_any_type());

      None;
    }
  | NamedExport(name, alias) => {
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (s => symbol_tbl.add(s, Util.generate_any_type()));

      None;
    };
