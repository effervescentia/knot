open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      switch (symbol_tbl.find(name)) {
      /* symbol exists, return the type reference */
      | Some(typ) when is_analyzed(typ) => typ

      /* symbol does not exist */
      | _ => raise(InvalidTypeReference)
      }

    | DotAccess(obj, key) =>
      switch (t_of(obj)) {
      /* symbol exists in object or module */
      | Object_t(members)
      | Module_t(_, members, _) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is generic and declares the property */
      | Generic_t(Some(Keyed_t(members))) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is generic and may allow the property to be declared */
      | Generic_t(t) =>
        let prop_typ = declared(any);

        (
          switch (t) {
          | None => Hashtbl.create(4)
          | Some(Keyed_t(members)) => members
          | _ => raise(InvalidDotAccess)
          }
        )
        |> (tbl => Hashtbl.add(tbl, key, prop_typ));

        prop_typ;
      | _ => raise(InvalidDotAccess)
      }

    | Execution(refr, args) =>
      switch (t_of(refr)) {
      /* symbol exists, using return type */
      | Function_t(_, return_type) => return_type

      /* symbol is a function */
      | Generic_t(Some(Callable_t(_, return_type))) =>
        /* TODO check arg types against rules */
        return_type

      /* symbol could be a function */
      | Generic_t(None) =>
        let arg_types = List.map(extract_ref, args);
        let typ = Callable_t(arg_types, inferred(any));

        refr =.= Generic_t(Some(typ));

        t_ref(refr);

      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  |:> promise;
