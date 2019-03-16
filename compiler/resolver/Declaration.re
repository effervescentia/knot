open Core;
open NestedHashtbl;

exception GenericConstant;

let (=<<) = (x, y) => {
  y(x);

  x;
};

let resolve = (symbol_tbl, name, (value, promise)) =>
  switch (value) {
  | ConstDecl(expr) =>
    switch (symbol_tbl.find(name)) {
    /* fail if any previous value declared by the same name */
    | Some(typ) when is_declared(typ) => raise(NameInUse(name))

    /* should not be used by anything by the time it's declared */
    | Some(_) => raise(UsedBeforeDeclaration(name))

    /* not in scope already */
    | None =>
      let expr_ref = t_ref(expr);

      switch (get_t(expr_ref^)) {
      /* expr returns a generic value */
      | Generic_t(_) => raise(GenericConstant)

      /* add value to symbol table */
      | _ => expr_ref =<< symbol_tbl.add(name) |:> promise
      };
    }

  | FunctionDecl(params, exprs) =>
    let param_types = List.map(extract_ref, params);
    let return_type =
      if (List.length(exprs) == 0) {
        declared(Nil_t);
      } else {
        List.nth(exprs, List.length(exprs) - 1) |> extract_ref;
      };
    let typ = Function_t(param_types, return_type);

    /* checking to see if function has been called already */
    switch (symbol_tbl.find(name)) {
    /* fail if any previous value declared by the same name */
    | Some(typ) when is_declared(typ) => raise(NameInUse(name))

    /* function matches its inferred type, replace return type and inferred type */
    | Some({
        contents:
          Inferred(
            Function_t(_, ret_type) as t |
            Generic_t(Some(Callable_t(_, ret_type))) as t,
          ),
      })
        when t =?? typ =>
      /* update any reference to the return type of this function */
      ret_type := return_type^;
      promise^ := Declared(typ);

    /* function matches its inferred type, replace inferred type */
    | Some({contents: Inferred(t)}) when t =?? typ =>
      promise^ := Declared(typ)

    /* function not in scope */
    | None => declared(typ) =<< symbol_tbl.add(name) |:> promise

    | _ => raise(InvalidTypeReference)
    };

  | _ => raise(TypeResolutionNotSupported)
  };
