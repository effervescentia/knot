open Core;

let resolve_declaration = (symbol_tbl, name, promise, typ) =>
  switch (NestedHashtbl.find(symbol_tbl, name)) {
  /* fail if any previous value declared by the same name */
  | Some(_) => throw_semantic(NameInUse(name))

  /* function not in scope */
  | None => typ =<< NestedHashtbl.add(symbol_tbl, name) <:= promise
  };

let resolve = (symbol_tbl, name, (value, promise)) =>
  switch (value) {
  | ConstDecl(expr) =>
    opt_type_ref(expr) |> resolve_declaration(symbol_tbl, name, promise)

  | FunctionDecl(params, exprs) =>
    let (param_types, return_type) =
      Function.resolve_callable(params, exprs);

    Function_t(param_types, return_type)
    |> resolve_declaration(symbol_tbl, name, promise);

  | ViewDecl(_, _, params, exprs) =>
    let (param_types, return_type) =
      Function.resolve_callable(params, exprs);

    /* only allow certain return types */
    switch (return_type) {
    | Number_t
    | String_t
    | Boolean_t
    | JSX_t
    | Nil_t => ()

    | _ => throw_semantic(UnrenderableReturnType(name))
    };

    View_t(param_types, return_type)
    |> resolve_declaration(symbol_tbl, name, promise);

  | StateDecl(params, props) =>
    let param_types = List.map(opt_type_ref, params);
    let props_tbl =
      List.map(((name, prop)) => (name, opt_type_ref(prop)), props)
      |> List.to_seq
      |> Hashtbl.of_seq;

    State_t(param_types, props_tbl)
    |> resolve_declaration(symbol_tbl, name, promise);

  | StyleDecl(params, rule_sets) =>
    let param_types = List.map(opt_type_ref, params);
    let classes =
      List.map(
        fst
        % (
          fun
          | ClassKey(s) => s
        ),
        rule_sets,
      );

    Style_t(param_types, classes)
    |> resolve_declaration(symbol_tbl, name, promise);
  };
