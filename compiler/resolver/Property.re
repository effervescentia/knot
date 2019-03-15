open Core;
open NestedHashtbl;

let resolve = (((name, type_def, default_val), promise)) =>
  (
    switch (type_def, default_val) {
    /* type declaration and default value do not match */
    | (
        Some((_, {contents: {contents: Resolved(l_type)}})),
        Some((_, {contents: {contents: Resolved(r_type)}})),
      )
        when l_type != r_type =>
      raise(DefaultValueTypeMismatch)

    /* either type declaration or default value available */
    | (
        Some((_, {contents: {contents: Resolved(_) | Synthetic(_)} as typ})),
        _,
      )
    | (
        _,
        Some((
          _,
          {contents: {contents: Resolved(_) | Synthetic(_)} as typ},
        )),
      ) =>
      Some(typ)

    /* no type declaration or default value */
    | (None, None) => Some(synthetic())

    | _ => raise(InvalidTypeReference)
    }
  )
  |::> promise;

let resolve_param = (symbol_tbl, ((name, _, _), _) as promise) =>
  if (resolve(promise)) {
    symbol_tbl.add(name, snd(promise));

    true;
  } else {
    false;
  };
