open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      switch (symbol_tbl.find(name)) {
      /* symbol exists, return the type reference */
      | Some({contents: Resolved(_) | Synthetic(_)}) as res => res

      /* symbol does not exist, add a synthetic type reference */
      | _ =>
        let typ = ref(Synthetic([]));
        symbol_tbl.add(name, typ);

        Some(typ);
      }

    | DotAccess((_, {contents: obj}), key) =>
      switch (obj^) {
      /* symbol exists in object or module */
      | Resolved(Object_t(members) | Module_t(_, members, _))
          when Hashtbl.mem(members, key) =>
        Some(Hashtbl.find(members, key))

      /* symbol is synthetic and declares the property */
      | Synthetic(rules) when V.has_key(rules, key) =>
        Some(V.get_prop_type(rules, key))

      /* symbol is synthetic and allows the property to be declared */
      | Synthetic(rules) when V.allows_key(rules, key) =>
        let prop_typ = ref(Synthetic([]));

        obj := Synthetic([HasProperty(key, prop_typ), ...rules]);

        Some(prop_typ);
      | _ => raise(InvalidDotAccess)
      }

    | Execution((_, {contents: refr}), args) =>
      switch (refr^) {
      /* symbol exists, using return type */
      | Resolved(Function_t(_, return_type)) => Some(return_type)

      /* symbol is a function */
      | Synthetic(rules) when V.is_callable(rules) =>
        /* TODO check arg types against rules */
        Some(V.get_return_type(rules))

      /* symbol could be a function */
      | Synthetic(rules) when V.allows_callable(rules) =>
        let typ = synthetic();
        refr := Synthetic([HasCallSignature([], typ), ...rules]);

        /* TODO check arg types against rules */
        Some(typ);
      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  |::> promise;
