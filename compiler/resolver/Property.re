open Core;
open NestedHashtbl;

let resolve:
  (NestedHashtbl.t(string, ref(eventual_type)), ast_property) => unit =
  (symbol_tbl, ((name, type_def, default_val), promise)) => {
    switch (symbol_tbl.find_local(name)) {
    | Some(_) => raise(NameInUse(name))
    | None => ()
    };

    (
      switch (type_def, default_val) {
      /* type declaration and default value do not match */
      | (Some(t_ref), Some(v_ref)) when t_of(t_ref) != t_of(v_ref) =>
        raise(DefaultValueTypeMismatch)

      /* either type declaration or default value l    able */
      | (Some((_, typ)), _)
      | (_, Some((_, typ))) => typ^

      /* no type declaration or default value */
      | (None, None) => declared(any)
      }
    )
    |:> promise;
  };

let resolve_param = (symbol_tbl, ((name, _, _), promise) as prop) => {
  resolve(symbol_tbl, prop);
  symbol_tbl.add(name, promise^);
};
