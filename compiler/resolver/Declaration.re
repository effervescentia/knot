open Core;
open NestedHashtbl;

exception UnrenderableReturnType;

let resolve_callable = (params, exprs) => {
  let param_types = List.map(opt_type_ref, params);
  let return_type =
    if (List.length(exprs) == 0) {
      Nil_t;
    } else {
      List.nth(exprs, List.length(exprs) - 1) |> opt_type_ref;
    };

  (param_types, return_type);
};

let resolve_declaration = (symbol_tbl, name, promise, typ) =>
  switch (symbol_tbl.find(name)) {
  /* fail if any previous value declared by the same name */
  | Some(_) => raise(NameInUse(name))

  /* function not in scope */
  | None => typ =<< symbol_tbl.add(name) <:= promise
  };

let resolve = (symbol_tbl, name, (value, promise)) =>
  switch (value) {
  | ConstDecl(expr) =>
    opt_type_ref(expr) |> resolve_declaration(symbol_tbl, name, promise)

  | FunctionDecl(params, exprs) =>
    let (param_types, return_type) = resolve_callable(params, exprs);

    Function_t(param_types, return_type)
    |> resolve_declaration(symbol_tbl, name, promise);

  | ViewDecl(_, _, params, exprs) =>
    let (param_types, return_type) = resolve_callable(params, exprs);

    /* only allow certain return types */
    switch (return_type) {
    | Number_t
    | String_t
    | Boolean_t
    | JSX_t
    | Nil_t => ()

    | _ => raise(UnrenderableReturnType)
    };

    View_t(param_types, return_type)
    |> resolve_declaration(symbol_tbl, name, promise);

  | _ => raise(TypeResolutionNotSupported)
  };
