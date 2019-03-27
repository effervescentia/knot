open Core;

let resolve = ((value, promise)) =>
  switch (value) {
  | Module(imports, stmts) =>
    let dependencies = ref([]);
    let members = Hashtbl.create(8);
    let main_declaration = ref(None);

    List.iter(
      fun
      | Import(module_, _) => dependencies := [module_, ...dependencies^],
      imports,
    );

    List.iter(
      fun
      | Declaration(name, decl) => {
          let decl_ref = opt_type_ref(decl);

          Hashtbl.add(members, name, decl_ref);
        }

      | Main(name, decl) => {
          let decl_ref = opt_type_ref(decl);

          Hashtbl.add(members, name, decl_ref);
          main_declaration := Some(decl_ref);
        },
      stmts,
    );

    Module_t(dependencies^, members, main_declaration^) <:= promise;
  };
