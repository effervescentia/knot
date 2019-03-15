open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, name, (value, promise)) =>
  (
    switch (value) {
    | ConstDecl((_, {contents: expr})) =>
      switch (symbol_tbl.find(name)) {
      /* should not be used by anything by the time it's declared */
      | Some(_) => raise(NameInUse(name))

      /* add to scope as normal */
      | None =>
        let typ = synthetic();
        symbol_tbl.add(name, typ);

        Some(typ);
      }
    | FunctionDecl(params, exprs) =>
      let param_types =
        List.map(
          ((_, typ)) =>
            switch (typ^) {
            | {contents: Resolved(_) | Synthetic(_)} as res => res
            | _ => raise(InvalidTypeReference)
            },
          params,
        );

      (
        if (List.length(exprs) == 0) {
          resolved(Function_t(param_types, ref(Resolved(Nil_t))));
        } else {
          let (_, last_expr) = List.nth(exprs, List.length(exprs) - 1);

          switch (last_expr^) {
          | {contents: Resolved(_) | Synthetic(_)} as return_typ =>
            let typ = ref(Resolved(Function_t(param_types, return_typ)));
            symbol_tbl.add(name, typ);

            Some(typ);
          | _ => raise(InvalidTypeReference)
          };
        }
      )
      |> (
        fun
        | Some(typ) as res =>
          /* TODO: check symbol table to see if type meets expectations */
          res
        | None => None
      );
    | _ => None
    }
  )
  |::> promise;
