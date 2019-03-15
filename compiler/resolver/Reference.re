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
        let typ = ref(Synthetic(Generic_t(None)));
        symbol_tbl.add(name, typ);

        Some(typ);
      }

    | DotAccess((_, {contents: obj}), key) =>
      switch (typeof(obj^)) {
      /* symbol exists in object or module */
      | Object_t(members)
      | Module_t(_, members, _) when Hashtbl.mem(members, key) =>
        Some(Hashtbl.find(members, key))

      /* symbol is generic and declares the property */
      | Generic_t(Some(Keyed_t(members))) when Hashtbl.mem(members, key) =>
        Some(Hashtbl.find(members, key))

      /* symbol is generic and may allow the property to be declared */
      | Generic_t(t) =>
        let prop_typ = ref(Resolved(Generic_t(None)));

        (
          switch (t) {
          | None => Hashtbl.create(4)
          | Some(Keyed_t(members)) => members
          | _ => raise(InvalidDotAccess)
          }
        )
        |> (tbl => Hashtbl.add(tbl, key, prop_typ));

        Some(prop_typ);
      | _ => raise(InvalidDotAccess)
      }

    | Execution((_, {contents: refr}), args) =>
      switch (typeof(refr^)) {
      /* symbol exists, using return type */
      | Function_t(_, return_type) => Some(return_type)

      /* symbol is a function */
      | Generic_t(Some(Callable_t(_, return_type))) =>
        /* TODO check arg types against rules */
        Some(return_type)

      /* symbol could be a function */
      | Generic_t(None) =>
        let typ = ref(Synthetic(Generic_t(None)));

        refr :=
          Synthetic(
            Generic_t(
              Some(
                Callable_t(
                  List.map(
                    ((_, {contents: arg_type})) =>
                      switch (arg_type^) {
                      | Resolved(_)
                      | Synthetic(_) => arg_type
                      | _ => raise(InvalidTypeReference)
                      },
                    args,
                  ),
                  typ,
                ),
              ),
            ),
          );

        Some(typ);

      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  |::> promise;
