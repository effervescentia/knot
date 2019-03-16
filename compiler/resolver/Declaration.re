open Core;
open NestedHashtbl;

/* HAS TODOS!!! */

let (=<<) = (x, y) => {
  y(x);

  x;
};

/* let func_type_matches = */

let resolve = (symbol_tbl, name, (value, promise)) => {
  /* fail if any previous value declared by the same name */
  switch (symbol_tbl.find(name)) {
  | Some(typ) when is_declared(typ) => raise(NameInUse(name))
  | Some(_) => ()
  | _ => ()
  };

  (
    switch (value) {
    | ConstDecl(expr) =>
      switch (symbol_tbl.find(name)) {
      /* should not be used by anything by the time it's declared */
      | Some(_) => raise(UsedBeforeDeclaration(name))

      /* add to scope */
      | None => t_ref(expr) =<< symbol_tbl.add(name)
      }

    | FunctionDecl(params, exprs) =>
      let param_types = List.map(extract_ref, params);
      let return_type =
        if (List.length(exprs) == 0) {
          declared(Nil_t);
        } else {
          List.nth(exprs, List.length(exprs) - 1) |> extract_ref;
        };

      /* checking to see if function has been called already */
      /* TODO: have to hoist synthetic members of scopes */
      switch (symbol_tbl.find(name)) {
      | Some({contents: Declared(typ) | Inferred(typ)}) =>
        /* TODO: check symbol table to see if type meets expectations */
        raise(InvalidTypeReference)
      | Some(_) => raise(InvalidTypeReference)
      | None =>
        declared(Function_t(param_types, return_type))
        =<< symbol_tbl.add(name)
      };

    | _ => raise(TypeResolutionNotSupported)
    }
  )
  |:> promise;
};
