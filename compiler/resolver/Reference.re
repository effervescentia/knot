open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      switch (symbol_tbl.find(name)) {
      /* symbol exists, return the type reference */
      | Some(typ) => typ

      /* symbol does not exist */
      | _ => raise(UsedBeforeDeclaration(name))
      }

    | DotAccess(obj, key) =>
      switch (opt_type_ref(obj)) {
      /* symbol exists in object or module */
      | Object_t(members)
      | Module_t(_, members, _) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      | _ => raise(InvalidDotAccess)
      }

    | Execution(refr, args) =>
      switch (opt_type_ref(refr)) {
      /* symbol is a function */
      | Function_t(_, return_type) => return_type

      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  <:= promise;
