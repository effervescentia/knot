open Core;

let resolve_expr = (name, type_def, default_val, promise) =>
  (
    switch (type_def, default_val) {
    /* type declaration and default value do not match */
    | (Some(t_ref), Some(v_ref))
        when opt_type_ref(t_ref) != opt_type_ref(v_ref) =>
      throw_semantic(DefaultValueTypeMismatch(name))

    /* either type declaration or default value provided */
    | (Some((_, typ)), _)
    | (_, Some((_, typ))) => unwrap_type(typ^)

    /* no type declaration or default value */
    | (None, None) => throw_semantic(UnableToInferType(name))
    }
  )
  <:= promise;

let resolve: (NestedHashtbl.t(string, member_type), ast_property) => unit =
  (symbol_tbl, ((name, type_def, default_val), promise)) => {
    switch (NestedHashtbl.find_local(symbol_tbl, name)) {
    | Some(_) => throw_semantic(NameInUse(name))
    | None => ()
    };

    resolve_expr(name, type_def, default_val, promise);
  };

let resolve_param = (symbol_tbl, ((name, _, _), promise) as prop) => {
  resolve(symbol_tbl, prop);

  NestedHashtbl.add(symbol_tbl, name, unwrap_type(promise^));
};
