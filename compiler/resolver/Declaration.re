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
        symbol_tbl.add(name, expr);

        Some(expr);
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
      let return_type =
        if (List.length(exprs) == 0) {
          ref(Resolved(Nil_t));
        } else {
          let (_, last_expr) = List.nth(exprs, List.length(exprs) - 1);

          switch (last_expr^) {
          | {contents: Resolved(_) | Synthetic(_)} as typ => typ
          | _ => raise(InvalidTypeReference)
          };
        };

      switch (symbol_tbl.find(name)) {
      | Some(typ) =>
        /* TODO: check symbol table to see if type meets expectations */
        raise(InvalidTypeReference)
      | None =>
        let typ = resolved(Function_t(param_types, return_type));
        symbol_tbl.add(name, typ);

        Some(typ);
      };
    | _ => None
    }
  )
  |::> promise;
