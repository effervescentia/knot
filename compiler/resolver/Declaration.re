open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, name, promise) =>
  (
    switch (fst(promise)) {
    | ConstDecl(expr) =>
      switch (Util.typeof(expr)) {
      | Some(typ) =>
        switch (symbol_tbl.find(name)) {
        | Some(_) => raise(NameInUse(name))
        | None =>
          symbol_tbl.add(name, typ);

          Some(ref(Resolved(typ)));
        }
      | None => None
      }
    | FunctionDecl(params, exprs) =>
      let synthetic = ref(false);

      let param_types =
        List.map(
          Util.typeof
          % (
            fun
            | Some(typ) => typ
            | None => {
                synthetic := true;
                Any_t;
              }
          ),
          params,
        );

      (
        if (List.length(exprs) == 0) {
          Some(Function_t(param_types, Nil_t));
        } else {
          switch (Util.typeof(List.nth(exprs, List.length(exprs) - 1))) {
          | Some(t) =>
            let typ = Function_t(param_types, t);
            symbol_tbl.add(name, typ);

            Some(typ);
          | None => None
          };
        }
      )
      |> (
        fun
        | Some(typ) =>
          Some(ref(synthetic^ ? Synthetic(typ, []) : Resolved(typ)))
        | None => None
      );
    | _ => None
    }
  )
  |::> snd(promise);
