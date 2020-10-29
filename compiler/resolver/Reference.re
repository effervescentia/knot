open Globals;

let resolve = (symbol_tbl, sidecar_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      NestedHashtbl.find(symbol_tbl, name)
      |!> CompilationError(SemanticError(UsedBeforeDeclaration(name)))

    | SidecarVariable(name) =>
      switch (sidecar_tbl) {
      /* symbol exists, return the type reference */
      | Some(tbl) when Hashtbl.mem(tbl, name) => Hashtbl.find(tbl, name)
      /* symbol does not exist */
      | Some(_) => throw_semantic(UsedBeforeDeclaration(name))
      | None => invariant(MissingSidecarScope)
      }

    | DotAccess(obj, key) =>
      switch (opt_type_ref(obj)) {
      /* symbol exists in object or module */
      | Object_t(members)
      | Module_t(_, members, _) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      | _ => throw_semantic(PropertyDoesNotExist(key))
      }

    | Execution(refr, args) =>
      switch (opt_type_ref(refr)) {
      /* symbol is a function */
      | Function_t(_, return_type)
      | Mutator_t(_, return_type) => return_type

      | _ => throw_semantic(ExecutingNonFunction)
      }
    }
  )
  <:= promise;
