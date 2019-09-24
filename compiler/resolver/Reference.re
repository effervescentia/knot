open Core;

let resolve = (symbol_tbl, sidecar_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      switch (NestedHashtbl.find(symbol_tbl, name)) {
      /* symbol exists, return the type reference */
      | Some(typ) => typ

      /* symbol does not exist */
      | _ => throw(UsedBeforeDeclaration(name))
      }

    | SidecarVariable(name) =>
      switch (sidecar_tbl) {
      | Some(tbl) =>
        if (Hashtbl.mem(tbl, name)) {
          /* symbol exists, return the type reference */
          Hashtbl.find(
            tbl,
            name,
          );
        } else {
          /* symbol does not exist */
          throw(UsedBeforeDeclaration(name));
        }
      | None => throw(MissingSidecarScope)
      }

    | DotAccess(obj, key) =>
      switch (opt_type_ref(obj)) {
      /* symbol exists in object or module */
      | Object_t(members)
      | Module_t(_, members, _) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      | _ => throw(InvalidDotAccess)
      }

    | Execution(refr, args) =>
      switch (opt_type_ref(refr)) {
      /* symbol is a function */
      | Function_t(_, return_type)
      | Mutator_t(_, return_type) => return_type

      | _ => throw(ExecutingNonFunction)
      }
    }
  )
  <:= promise;
