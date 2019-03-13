open Core;

let resolve = promise =>
  (
    switch (fst(promise)) {
    | Module(stmts) =>
      let dependencies = ref([]);
      let declarations = Hashtbl.create(8);
      let main_declaration = ref(None);

      if (List.for_all(
            fun
            | Import(module_, imports) => {
                dependencies := [module_, ...dependencies^];

                List.for_all(Util.is_resolved, imports);
              }
            | Declaration(name, decl) =>
              switch (Util.typeof(decl)) {
              | Some(typ) =>
                Hashtbl.add(declarations, name, typ);

                true;
              | None => false
              }
            | Main(name, decl) =>
              switch (Util.typeof(decl)) {
              | Some(typ) =>
                Hashtbl.add(declarations, name, typ);
                main_declaration := Some(typ);

                true;
              | None => false
              },
            stmts,
          )) {
        Some(Module_t(dependencies^, declarations, main_declaration^));
      } else {
        None;
      };
    }
  )
  |%> resolve_iff(promise);
