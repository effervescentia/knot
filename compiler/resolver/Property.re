open Core;
open NestedHashtbl;

let resolve: ast_property => unit =
  (((name, type_def, default_val), promise)) =>
    (
      switch (type_def, default_val) {
      /* type declaration and default value do not match */
      | (
          Some((_, {contents: {contents: Declared(l_type)}})),
          Some((_, {contents: {contents: Declared(r_type)}})),
        )
          when l_type != r_type =>
        raise(DefaultValueTypeMismatch)

      /* either type declaration or default value available */
      | (Some((_, typ)), _)
      | (_, Some((_, typ))) =>
        let typ_ref = typ^;

        if (is_declared(typ_ref)) {
          Some(typ_ref);
        } else {
          raise(InvalidTypeReference);
        };

      /* no type declaration or default value */
      | (None, None) => Some(declared(any))
      }
    )
    |::> promise;

let resolve_param = (symbol_tbl, ((name, _, _), promise) as prop) => {
  resolve(prop);
  symbol_tbl.add(name, promise^);
};
