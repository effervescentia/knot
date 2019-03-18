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
      | _ => raise(InvalidTypeReference)
      }

    | DotAccess(obj, key) =>
      switch (opt_type_ref(obj) |> typeof_ref) {
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
          /* symbol is currently of type any */
          | None => Hashtbl.create(4)

          /* symbol is a keyed type */
          | Some(Keyed_t(members)) => members

          /* type does not support property access */
          | _ => raise(InvalidDotAccess)
          }
        )
        |> (tbl => Hashtbl.add(tbl, key, prop_typ));

        prop_typ;

      | _ => raise(InvalidDotAccess)
      }

    | Execution(refr, args) =>
      (
        switch ((snd(refr))^) {
        /* symbol has been analyzed */
        | Some(x) =>
          switch (typeof_ref(x)) {
          /* symbol is a function */
          | Function_t(_, return_type) => Some(return_type)

          /* symbol is callable */
          | Generic_t(Some(Callable_t(_, return_type))) =>
            /* TODO check arg types against rules */
            Some(return_type)

          /* symbol could be a function */
          | Generic_t(None) => None

          | _ => raise(ExecutingNonFunction)
          }

        /* symbol has not been analyzed */
        | None => None
        }
      )
      |> (
        fun
        | Some(res) => res
        | None => {
            let arg_types = List.map(opt_type_ref, args);
            let ret_type = inferred(any);
            let typ = Callable_t(arg_types, ret_type);

            snd(refr) =.= generic(typ);

            ret_type;
          }
      )
    }
  )
  <:= promise;
