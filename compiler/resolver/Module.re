open Core;

let resolve = ((value, promise)) =>
  switch (value) {
  | Module(stmts) =>
    let dependencies = ref([]);
    let members = Hashtbl.create(8);
    let main_declaration = ref(None);

    List.iter(
      fun
      | Import(module_, _) => dependencies := [module_, ...dependencies^]

      | Declaration(name, decl) => {
          let decl_ref = opt_type_ref(decl);

          switch (decl_ref^) {
          | Declared(_) => Hashtbl.add(members, name, decl_ref)
          | _ => raise(ModuleTypeIncomplete)
          };
        }

      | Main(name, decl) => {
          let arg_ref = opt_type_ref(decl);

          Hashtbl.add(members, name, arg_ref);
          main_declaration := Some(arg_ref);
        },
      stmts,
    );

    declared(Module_t(dependencies^, members, main_declaration^)) <:= promise;
  };
