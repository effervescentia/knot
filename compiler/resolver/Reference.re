open Core;
open NestedHashtbl;

let resolve_keyed_generic = (obj_ref, key, typ, tag) => {
  let prop_typ = ref(tag(any));

  (
    switch (typ) {
    /* symbol is currently of type any */
    | None =>
      let tbl = Hashtbl.create(4);

      obj_ref := tag(generic(Keyed_t(tbl)));

      tbl;

    /* symbol is a keyed type */
    | Some(Keyed_t(members)) => members

    /* type does not support property access */
    | _ => raise(InvalidDotAccess)
    }
  )
  |> (tbl => Hashtbl.add(tbl, key, prop_typ));

  prop_typ;
};

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
      let obj_ref = opt_type_ref(obj);

      switch (obj_ref^) {
      /* symbol exists in object or module */
      | Defined(Object_t(members))
      | Declared(Object_t(members))
      | Inferred(Object_t(members))
      | Defined(Module_t(_, members, _))
      | Declared(Module_t(_, members, _))
      | Inferred(Module_t(_, members, _)) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is generic and declares the property */
      | Defined(Generic_t(Some(Keyed_t(members))))
      | Declared(Generic_t(Some(Keyed_t(members))))
      | Inferred(Generic_t(Some(Keyed_t(members))))
          when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is defined as generic and may allow the type to change */
      | Defined(Generic_t(t)) =>
        resolve_keyed_generic(obj_ref, key, t, x => Defined(x))

      /* symbol is inferred as generic and may allow the type to change */
      | Inferred(Generic_t(t)) =>
        resolve_keyed_generic(obj_ref, key, t, x => Inferred(x))

      | _ => raise(InvalidDotAccess)
      };

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