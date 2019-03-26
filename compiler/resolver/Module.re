open Core;

let resolve_delc = (add, name, decl_ref) =>
  switch (decl_ref^) {
  | (_, Declared(_)) => add(name, decl_ref)
  | _ => raise(DeclarationNotFound(name))
  };

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

          resolve_delc(Hashtbl.add(members), name, decl_ref);
        }

      | Main(name, decl) => {
          let decl_ref = opt_type_ref(decl);

          resolve_delc(Hashtbl.add(members), name, decl_ref);
          main_declaration := Some(decl_ref);
        },
      stmts,
    );

    declared(Module_t(dependencies^, members, main_declaration^)) <:= promise;
  };
