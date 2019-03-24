open Core;
open NestedHashtbl;

let resolve_callable = (params, exprs) => {
  let param_types = List.map(opt_type_ref, params);
  let return_type =
    if (List.length(exprs) == 0) {
      declared(Nil_t);
    } else {
      List.nth(exprs, List.length(exprs) - 1) |> opt_type_ref;
    };

  (param_types, return_type);
};

let resolve_inferrable = (symbol_tbl, name, typ, promise, f) =>
  switch (symbol_tbl.find(name)) {
  /* fail if any previous value declared by the same name */
  | Some(t) when is_declared(t) => raise(NameInUse(name))

  | Some(t) =>
    switch (t^) {
    /* type is inferred as any, replace inferred type */
    | Inferred(Generic_t(None)) => promise =:= typ

    /* use type handler */
    | Inferred(res) => f(res)

    | _ => raise(InvalidTypeReference)
    }

  /* function not in scope */
  | None => declared(typ) =<< symbol_tbl.add(name) <:= promise
  };

let resolve = (symbol_tbl, name, (value, promise)) =>
  switch (value) {
  | ConstDecl(expr) =>
    switch (symbol_tbl.find(name)) {
    /* fail if any previous value exists by the same name */
    | Some(typ) => raise(NameInUse(name))

    /* not in scope already */
    | None =>
      let expr_ref = opt_type_ref(expr);

      switch (typeof_ref(expr_ref)) {
      /* expr returns a generic value */
      | Generic_t(_) => raise(GenericConstant)

      /* add value to symbol table */
      | _ => expr_ref =<< symbol_tbl.add(name) <:= promise
      };
    }

  | FunctionDecl(params, exprs) =>
    let (param_types, return_type) = resolve_callable(params, exprs);
    let typ = Function_t(param_types, return_type);

    (
      fun
      /* function matches its inferred type, replace return type and inferred type */
      | Function_t(args, ret) as t
      | Generic_t(Some(Callable_t(args, ret))) as t when t =?? typ => {
          /* update any references to this function */
          List.iteri((i, arg) => arg := (List.nth(param_types, i))^, args);
          ret := return_type^;
          promise =:= t;
        }

      | _ => raise(InvalidTypeReference)
    )
    /* checking to see if function has been called already */
    |> resolve_inferrable(symbol_tbl, name, typ, promise);

  | ViewDecl(_, _, params, exprs) =>
    let (param_types, return_type) = resolve_callable(params, exprs);

    /* only allow certain return types */
    switch (typeof(return_type^)) {
    | Number_t
    | String_t
    | Boolean_t
    | JSX_t
    | Nil_t => ()

    | _ => raise(InvalidTypeReference)
    };

    let typ = View_t(param_types, return_type);

    (
      fun
      /* view matches its inferred type, replace return type and inferred type */
      | View_t(args, ret) as t when t =?? typ => {
          /* update any reference to the return type of this view */
          List.iteri((i, arg) => arg := (List.nth(param_types, i))^, args);
          ret := return_type^;
          promise =:= t;
        }

      | _ => raise(InvalidTypeReference)
    )
    |> resolve_inferrable(symbol_tbl, name, typ, promise);

  | _ => raise(TypeResolutionNotSupported)
  };
