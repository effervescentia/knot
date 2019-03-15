open Core;
open NestedHashtbl;

let resolve = (((name, type_def, default_val), promise)) =>
  (
    switch (type_def, default_val) {
    | (
        Some((_, {contents: {contents: Resolved(l_type)}})),
        Some((_, {contents: {contents: Resolved(r_type)}})),
      )
        when l_type !== r_type =>
      raise(DefaultValueTypeMismatch)
    | (None, None) => Some(ref(Synthetic(Any_t, [])))
    | (Some((_, {contents: {contents: Resolved(typ)}})), _)
    | (_, Some((_, {contents: {contents: Resolved(typ)}}))) =>
      Some(ref(Resolved(typ)))
    | _ => None
    }
  )
  |::> promise;

let resolve_param = (symbol_tbl, ((name, _, _), _) as promise) =>
  if (resolve(promise)) {
    symbol_tbl.add(name, Util.get_type(promise));
    true;
  } else {
    false;
  };
