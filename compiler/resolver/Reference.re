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
      | _ => raise(UsedBeforeDeclaration(name))
      }

    | DotAccess(obj, key) =>
      let obj_ref = opt_type_ref(obj);

      switch (obj_ref^) {
      /* symbol exists in object or module */
      | (Object_t(members), _)
      | (Module_t(_, members, _), _) when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is generic and declares the property */
      | (Generic_t(Some(Keyed_t(members))), _)
          when Hashtbl.mem(members, key) =>
        Hashtbl.find(members, key)

      /* symbol is generic and allows the type to change */
      | (Generic_t(t), Declared(true) as t_ctx | Expected as t_ctx) =>
        resolve_keyed_generic(obj_ref, key, t, x => (x, t_ctx))

      | _ => raise(InvalidDotAccess)
      };

    | Execution(refr, args) =>
      (
        switch ((snd(refr))^) {
        /* symbol has been analyzed */
        | Some(x) =>
          switch (fst(x^)) {
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
            let ret_type = declared_mut(any);
            let typ = Callable_t(arg_types, ret_type);

            snd(refr) =@= (generic(typ), Expected);

            ret_type;
          }
      )
    }
  )
  <:= promise;
