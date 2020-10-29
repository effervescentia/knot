open Globals;

let resolve_prop = (sidecar_tbl, name, (value, promise)) =>
  switch (value) {
  | `Property(type_def, default_value) =>
    if (Hashtbl.mem(sidecar_tbl, name)) {
      throw_semantic(NameInUse(name));
    } else {
      Property.resolve_expr(name, type_def, default_value, promise);

      unwrap_type(promise^) |> Hashtbl.add(sidecar_tbl, name);
    }
  };

let resolve_method = (sidecar_tbl, name, (value, promise)) =>
  switch (value) {
  | `Getter(params, exprs)
  | `Mutator(params, exprs) =>
    if (Hashtbl.mem(sidecar_tbl, name)) {
      throw_semantic(NameInUse(name));
    } else {
      let (param_types, return_type) =
        Function.resolve_callable(params, exprs);

      Mutator_t(param_types, return_type)
      =<< Hashtbl.add(sidecar_tbl, name)
      <:= promise;
    }
  };
