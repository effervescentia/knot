open Core;

let resolve_prop = (sidecar_tbl, name, (value, promise)) =>
  switch (value) {
  | `Property(type_def, default_value) =>
    if (Hashtbl.mem(sidecar_tbl, name)) {
      raise(NameInUse(name));
    } else {
      Property.resolve_expr(type_def, default_value, promise);

      unwrap_type(promise^) |> Hashtbl.add(sidecar_tbl, name);
    }
  };
