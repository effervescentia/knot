open Core;

exception ModuleTypeIncomplete;

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | Module(stmts) =>
      let dependencies = ref([]);
      let members = Hashtbl.create(8);
      let main_declaration = ref(None);

      List.iter(
        fun
        | Import(module_, imports) =>
          dependencies := [module_, ...dependencies^]

        | Declaration(name, (_, decl)) =>
          switch (decl^ ^) {
          | Resolved(_) => Hashtbl.add(members, name, decl^)
          | _ => raise(ModuleTypeIncomplete)
          }

        | Main(name, (_, decl)) =>
          switch (decl^ ^) {
          | Resolved(_)
          | Synthetic(_) =>
            Hashtbl.add(members, name, decl^);
            main_declaration := Some(decl^);

          | _ => raise(ModuleTypeIncomplete)
          },
        stmts,
      );

      let typ = Module_t(dependencies^, members, main_declaration^);
      Some(resolved(typ));
    }
  )
  |::> promise;
